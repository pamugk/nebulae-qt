#include "gogapiclient.h"

#include <QNetworkRequest>
#include <QNetworkReply>
#include <QOAuthHttpServerReplyHandler>
#include <QProcessEnvironment>
#include <QUrlQuery>

api::GogApiClient::GogApiClient(AuthDataStorage *tokenStorage, QObject *parent)
    : QObject{parent},
      api(),
      client(this),
      oauth(this),
      refreshingToken(false),
      userId()
{
    connect(&client, &QNetworkAccessManager::finished, this, [this](QNetworkReply *reply)
    {
        if (reply->error() == QNetworkReply::AuthenticationRequiredError && !refreshingToken)
        {
            refreshingToken = true;
            oauth.refreshTokens();
        }
    });
    auto environment = QProcessEnvironment::systemEnvironment();

    auto replyHandler = new QOAuthHttpServerReplyHandler(6543, &client);
    connect(replyHandler, &QOAuthHttpServerReplyHandler::tokensReceived,
            this, [this, tokenStorage](const QVariantMap &data)
    {
        QString token = data[QLatin1StringView("access_token")].toString();
        api.setBearerToken(token.toLatin1());
        refreshingToken = false;
        userId = data[QLatin1StringView("user_id")].toString();
        QVariantMap savedData(
        {
            std::pair(QLatin1StringView("access_token"), token),
            std::pair(QLatin1StringView("refresh_token"), data[QLatin1StringView("refresh_token")]),
            std::pair(QLatin1StringView("user_id"), userId),
        });
        tokenStorage->setAuthData(savedData);
    });
    oauth.setReplyHandler(replyHandler);
    connect(tokenStorage, &AuthDataStorage::authDataAcquired,
            this, [this](const QVariantMap &data)
    {
        QString token = data[QLatin1StringView("access_token")].toString();
        oauth.setToken(token);
        oauth.setRefreshToken(data[QLatin1StringView("refresh_token")].toString());
        if (data.contains(QLatin1StringView("user_id")))
        {
            userId = data[QLatin1StringView("user_id")].toString();
        }
        else
        {
            userId = QString();
        }

        bool authenticated = !token.isEmpty() && !token.isNull();
        if (authenticated)
        {
            api.setBearerToken(token.toLatin1());
        }
        else
        {
            api.clearBearerToken();
        }

        emit this->authenticated(authenticated);
    });
    tokenStorage->getAuthData();
    oauth.setClientIdentifier(environment.value(QLatin1StringView("GOG_CLIENT_ID")));
    oauth.setClientIdentifierSharedKey(environment.value(QLatin1StringView("GOG_CLIENT_SECRET")));
    oauth.setAuthorizationUrl(QUrl(QLatin1StringView("https://auth.gog.com/auth")));
    oauth.setTokenUrl(QUrl(QLatin1StringView("https://auth.gog.com/token")));
    oauth.setModifyParametersFunction([&](QAbstractOAuth::Stage stage, QMultiMap<QString, QVariant> *parameters)
    {
        switch (stage)
        {
            case QAbstractOAuth::Stage::RequestingTemporaryCredentials:
            case QAbstractOAuth::Stage::RequestingAuthorization:
            {
                parameters->insert(QLatin1StringView("layout"), QLatin1StringView("client2"));
            }
            case QAbstractOAuth::Stage::RequestingAccessToken:
            {
                parameters->remove(QLatin1StringView("redirect_uri"));
                parameters->insert(QLatin1StringView("redirect_uri"), QLatin1StringView("https://embed.gog.com/on_login_success?origin=client"));
                break;
            }
            case QAbstractOAuth::Stage::RefreshingAccessToken:
                break;
        }
    });

    connect(&oauth, &QOAuth2AuthorizationCodeFlow::statusChanged,
            this, [this](QAbstractOAuth::Status status)
    {
        if (status == QAbstractOAuth::Status::Granted)
        {
            refreshingToken = false;
            emit authenticated(true);
        }
        else if (status == QAbstractOAuth::Status::NotAuthenticated)
        {
            refreshingToken = false;
            logout();
        }
    });
    connect(&oauth, &QOAuth2AuthorizationCodeFlow::authorizeWithBrowser, this, &GogApiClient::authorize);

    connect(tokenStorage, &AuthDataStorage::authDataRequested, this, [this, tokenStorage]()
    {
        QVariantMap data;
        if (!oauth.token().isEmpty())
        {
            data[QLatin1StringView("access_token")] = oauth.token();
            data[QLatin1StringView("refresh_token")] = oauth.refreshToken();
            data[QLatin1StringView("user_id")] = userId;
        }
        tokenStorage->setAuthData(data);
    });
}

QString api::GogApiClient::currentUserId() const
{
    return userId;
}

bool api::GogApiClient::isAuthenticated()
{
    return !oauth.token().isEmpty() && !oauth.token().isNull();
}

QNetworkReply *api::GogApiClient::getAchievements()
{
    QNetworkRequest request = api.createRequest();
    request.setUrl(QUrl(QLatin1StringView("https://gameplay.gog.com/users/%1/sessions").arg(userId)));
    return client.get(request);
}

QNetworkReply *api::GogApiClient::getAnything(const QString &url)
{
    QNetworkRequest request = api.createRequest();
    request.setUrl(QUrl(url));
    return client.get(request);
}

QNetworkReply *api::GogApiClient::getCatalogProductInfo(const QString &id, const QString &locale)
{
    QUrlQuery parameters(
    {
        std::pair(QLatin1StringView("locale"), locale),
    });
    QUrl url(QLatin1StringView("https://api.gog.com/v2/games/%1").arg(id));
    url.setQuery(parameters);
    QNetworkRequest request = api.createRequest();
    request.setUrl(url);
    return client.get(request);
}

QNetworkReply *api::GogApiClient::getCurrentUser()
{
    return getUser(userId);
}

QNetworkReply *api::GogApiClient::getCurrentUserGameTimeStatistics()
{
    QNetworkRequest request = api.createRequest();
    request.setUrl(QUrl(QLatin1StringView("https://gameplay.gog.com/users/%1/external_game_time_stats").arg(userId)));
    return client.get(request);
}

QNetworkReply *api::GogApiClient::getCurrentUserPlatformAchievements(const QString &platform, const QString &pageToken)
{
    QUrlQuery parameters(
    {
        std::pair(QLatin1StringView("platform"), platform),
    });
    if (!pageToken.isEmpty())
    {
        parameters.addQueryItem(QLatin1StringView("page_token"), pageToken);
    }

    QUrl url(QLatin1StringView("https://gameplay.gog.com/users/%1/external_achievements").arg(userId));
    url.setQuery(parameters);
    QNetworkRequest request = api.createRequest();
    request.setUrl(url);
    return client.get(request);
}

QNetworkReply *api::GogApiClient::getCurrentUserPlatformReleaseAchievements(const QString &platformId, const QString &platformReleaseId, const QString &pageToken)
{
    QUrl url(QLatin1StringView("https://gameplay.gog.com/external_releases/%1_%2/users/%3/achievements").arg(platformId, platformReleaseId, userId));
    if (!pageToken.isEmpty())
    {
        QUrlQuery parameters(
        {
            std::pair(QLatin1StringView("page_token"), pageToken),
        });
        url.setQuery(parameters);
    }
    QNetworkRequest request = api.createRequest();
    request.setUrl(url);
    return client.get(request);
}

QNetworkReply *api::GogApiClient::getCurrentUserPlatformReleaseGameTimeStatistics(const QString &platformId, const QString &platformReleaseId)
{
    QNetworkRequest request = api.createRequest();
    request.setUrl(QUrl(QLatin1StringView("https://gameplay.gog.com/external_releases/%1_%2/users/%3/sessions").arg(platformId, platformReleaseId, userId)));
    return client.get(request);
}

QNetworkReply *api::GogApiClient::getCurrentUserReleases()
{
    QNetworkRequest request = api.createRequest();
    request.setUrl(QUrl(QLatin1StringView("https://galaxy-library.gog.com/users/%1/releases").arg(userId)));
    return client.get(request);
}

QNetworkReply *api::GogApiClient::getGame(const QString &id)
{
    QNetworkRequest request = api.createRequest();
    request.setUrl(QUrl(QLatin1StringView("https://gamesdb.gog.com/games/%1").arg(id)));
    return client.get(request);
}

QNetworkReply *api::GogApiClient::getNews(unsigned short pageToken, const QString &locale,
                                          unsigned char limit)
{
    QUrlQuery parameters(
    {
        std::pair(QLatin1StringView("language_code"), locale),
        std::pair(QLatin1StringView("page_token"), QString::number(pageToken)),
        std::pair(QLatin1StringView("limit"), QString::number(limit)),
    });
    QUrl url(QLatin1StringView("https://api.gog.com/news"));
    url.setQuery(parameters);
    QNetworkRequest request = api.createRequest();
    request.setUrl(url);
    return client.get(request);
}

QNetworkReply *api::GogApiClient::getNowOnSale(const QString &locale, const QString &countryCode, const QString &currencyCode)
{
    QUrlQuery parameters(
    {
        std::pair(QLatin1StringView("locale"), locale),
        std::pair(QLatin1StringView("countryCode"), countryCode),
        std::pair(QLatin1StringView("currencyCode"), currencyCode),
    });
    QUrl url(QLatin1StringView("https://api.gog.com/now_on_sale"));
    url.setQuery(parameters);
    QNetworkRequest request = api.createRequest();
    request.setUrl(url);
    return client.get(request);
}

QNetworkReply *api::GogApiClient::getNowOnSaleSection(const QString &sectionId)
{
    QNetworkRequest request = api.createRequest();
    request.setUrl(QUrl(QLatin1StringView("https://api.gog.com/now_on_sale/%1").arg(sectionId)));
    return client.get(request);
}

QNetworkReply *api::GogApiClient::getOrdersHistory(const OrderFilter &filter, unsigned short page)
{
    QUrlQuery parameters(
    {
        std::pair(QLatin1StringView("canceled"), QLatin1StringView(filter.cancelled ? "1" : "0")),
        std::pair(QLatin1StringView("completed"), QLatin1StringView(filter.completed ? "1" : "0")),
        std::pair(QLatin1StringView("in_progress"), QLatin1StringView(filter.inProgress ? "1" : "0")),
        std::pair(QLatin1StringView("not_redeemed"), QLatin1StringView(filter.notRedeemed ? "1" : "0")),
        std::pair(QLatin1StringView("pending"), QLatin1StringView(filter.pending ? "1" : "0")),
        std::pair(QLatin1StringView("redeemed"), QLatin1StringView(filter.redeemed ? "1" : "0")),
        std::pair(QLatin1StringView("page"), QString::number(page)),
    });
    if (!filter.query.isEmpty())
    {
        parameters.addQueryItem(QLatin1StringView("search"), filter.query);
    }

    QUrl url(QLatin1StringView("https://embed.gog.com/account/settings/orders/data"));
    url.setQuery(parameters);
    QNetworkRequest request = api.createRequest();
    request.setUrl(url);
    return client.get(request);
}

QNetworkReply *api::GogApiClient::getOwnedLicensesIds()
{
    QNetworkRequest request = api.createRequest();
    request.setUrl(QUrl(QLatin1StringView("https://menu.gog.com/v1/account/licences")));
    return client.get(request);
}

QNetworkReply *api::GogApiClient::getOwnedProducts(const QString &query, const QString &order, unsigned short page)
{
    QUrlQuery parameters(
    {
        std::pair(QLatin1StringView("hiddenFlag"), QLatin1StringView("0")),
        std::pair(QLatin1StringView("mediaType"), QLatin1StringView("1")),
        std::pair(QLatin1StringView("sortBy"), order),
        std::pair(QLatin1StringView("page"), QString::number(page)),
    });
    if (!query.isEmpty())
    {
        parameters.addQueryItem(QLatin1StringView("search"), query);
    }
    QUrl url(QLatin1StringView("https://embed.gog.com/account/getFilteredProducts"));
    url.setQuery(parameters);
    QNetworkRequest request = api.createRequest();
    request.setUrl(url);
    return client.get(request);
}

QNetworkReply *api::GogApiClient::getPlatformRelease(const QString &platformId, const QString &platformReleaseId)
{
    QNetworkRequest request = api.createRequest();
    request.setUrl(QUrl(QLatin1StringView("https://gamesdb.gog.com/platforms/%1/external_releases/%2").arg(platformId, platformReleaseId)));
    return client.get(request);
}

QNetworkReply *api::GogApiClient::getPlatformReleaseAchievements(const QString &platformId, const QString &platformReleaseId,
                                                                 const QString &locale)
{
    QUrlQuery parameters(
    {
        std::pair(QLatin1StringView("locale"), locale)
    });
    QUrl url(QLatin1StringView("https://gameplay.gog.com/external_releases/%1_%2/achievements").arg(platformId, platformReleaseId));
    url.setQuery(parameters);
    QNetworkRequest request = api.createRequest();
    request.setUrl(url);
    return client.get(request);
}

QNetworkReply *api::GogApiClient::getProductAchievements(const QString &productId)
{
    QNetworkRequest request = api.createRequest();
    request.setUrl(QUrl(QLatin1StringView("https://gameplay.gog.com/clients/%1/users/%2/sessions").arg(productId, userId)));
    return client.get(request);
}

QNetworkReply *api::GogApiClient::getProductAverageRating(const QString &productId, const QString &reviewer)
{
    QUrl url(QLatin1StringView("https://reviews.gog.com/v1/products/%1/averageRating").arg(productId));
    if (!reviewer.isEmpty())
    {
        QUrlQuery parameters(
        {
            std::pair(QLatin1StringView("reviewer"), reviewer),
        });
        url.setQuery(parameters);
    }
    QNetworkRequest request = api.createRequest();
    request.setUrl(url);
    return client.get(request);
}

QNetworkReply *api::GogApiClient::getProductPrices(const QString &productId, const QString &countryCode)
{
    QUrlQuery parameters(
    {
        std::pair(QLatin1StringView("countryCode"), countryCode),
    });
    QUrl url(QLatin1StringView("https://api.gog.com/products/%1/prices").arg(productId));
    url.setQuery(parameters);
    QNetworkRequest request = api.createRequest();
    request.setUrl(url);
    return client.get(request);
}

QNetworkReply *api::GogApiClient::getProductRecommendationsPurchasedTogether(const QString &productId,
                                                                            const QString &countryCode,
                                                                            const QString &currency,
                                                                            unsigned char limit)
{
    QUrlQuery parameters(
    {
        std::pair(QLatin1StringView("country_code"), countryCode),
        std::pair(QLatin1StringView("currency"), currency),
        std::pair(QLatin1StringView("limit"), QString::number(limit)),
    });
    QUrl url(QLatin1StringView("https://recommendations-api.gog.com/v1/recommendations/purchased_together/%1").arg(productId));
    url.setQuery(parameters);
    QNetworkRequest request = api.createRequest();
    request.setUrl(url);
    return client.get(request);
}

QNetworkReply *api::GogApiClient::getProductRecommendationsSimilar(const QString &productId,
                                                                  const QString &countryCode,
                                                                  const QString &currency,
                                                                  unsigned char limit)
{
    QUrlQuery parameters(
    {
        std::pair(QLatin1StringView("country_code"), countryCode),
        std::pair(QLatin1StringView("currency"), currency),
        std::pair(QLatin1StringView("limit"), QString::number(limit)),
    });
    QUrl url(QLatin1StringView("https://recommendations-api.gog.com/v1/recommendations/similar/%1").arg(productId));
    url.setQuery(parameters);
    QNetworkRequest request = api.createRequest();
    request.setUrl(url);
    return client.get(request);
}

QNetworkReply *api::GogApiClient::getProductReviews(const QString &productId,
                                                    const ReviewFilters &filters,
                                                    const SortOrder &order,
                                                    unsigned short limit, unsigned short page)
{
    QUrlQuery parameters(
    {
        std::pair(QLatin1StringView("limit"), QString::number(limit)),
        std::pair(QLatin1StringView("order"), QLatin1StringView("%1:%2").arg(QLatin1StringView(order.ascending ? "asc" : "desc"), order.field)),
        std::pair(QLatin1StringView("page"), QString::number(page)),
    });
    if (filters.lastDays.has_value())
    {
        QDateTime searchStart = QDateTime::currentDateTime().addDays(-1 * filters.lastDays.value());
        parameters.addQueryItem(QLatin1StringView("date"), QLatin1StringView("gte:%1").arg(searchStart.toString(Qt::DateFormat::ISODate)));
    }
    QStringList selectedLanguages;
    if (filters.otherLanguages)
    {
        for (const QString &language : std::as_const(filters.allLanguages))
        {
            if (!filters.languages.contains(language))
            {
                selectedLanguages << language;
            }
        }
    }
    else
    {
        selectedLanguages.reserve(filters.languages.count());
        for (const QString &language : std::as_const(filters.languages))
        {
            selectedLanguages << language;
        }
    }
    if (!selectedLanguages.isEmpty())
    {
        parameters.addQueryItem(QLatin1StringView("language"),
                           QLatin1StringView("%1:%2")
                            .arg(QLatin1StringView(filters.otherLanguages ? "not_in" : "in"), selectedLanguages.join(',')));
    }
    if (filters.reviewedByOwner.has_value())
    {
        parameters.addQueryItem(QLatin1StringView("reviewer"),
                           QLatin1StringView(filters.reviewedByOwner.value()
                            ? "in:verified_owner"
                            : "not_in:verified_owner"));
    }
    if (filters.reviewedDuringDevelopment.has_value())
    {
        parameters.addQueryItem(QLatin1StringView("version"),
                           QLatin1StringView(filters.reviewedDuringDevelopment.value()
                            ? "in:in_development"
                            : "not_in:in_development"));
    }

    QUrl url(QLatin1StringView("https://reviews.gog.com/v1/products/%1/reviews").arg(productId));
    url.setQuery(parameters);
    QNetworkRequest request = api.createRequest();
    request.setUrl(url);
    return client.get(request);
}

QNetworkReply *api::GogApiClient::getSeriesGames(unsigned long long seriesId)
{
    QUrlQuery parameters(
    {
        std::pair(QLatin1StringView("seriesId"), QString::number(seriesId)),
    });
    QUrl url(QLatin1StringView("https://api.gog.com/v2/games"));
    url.setQuery(parameters);
    QNetworkRequest request = api.createRequest();
    request.setUrl(url);
    return client.get(request);
}

QNetworkReply *api::GogApiClient::getRelease(const QString &id)
{
    QNetworkRequest request = api.createRequest();
    request.setUrl(QUrl(QLatin1StringView("https://gamesdb.gog.com/releases/%1").arg(id)));
    return client.get(request);
}

QNetworkReply *api::GogApiClient::getSeriesPrices(unsigned long long seriesId,
                                                  const QString &countryCode,
                                                  const QString &currencyCode)
{
    QUrlQuery parameters(
    {
        std::pair(QLatin1StringView("seriesId"), QString::number(seriesId)),
        std::pair(QLatin1StringView("countryCode"), countryCode),
        std::pair(QLatin1StringView("currency"), currencyCode),
    });
    QUrl url(QLatin1StringView("https://api.gog.com/products/prices"));
    url.setQuery(parameters);
    QNetworkRequest request = api.createRequest();
    request.setUrl(url);
    return client.get(request);
}

QNetworkReply *api::GogApiClient::getStoreSection(const QString &pageHex, const QString &id, const QString &locale, const QString &countryCode, const QString &currencyCode)
{
    QUrlQuery parameters(
    {
        std::pair(QLatin1StringView("locale"), locale),
        std::pair(QLatin1StringView("countryCode"), countryCode),
        std::pair(QLatin1StringView("currencyCode"), currencyCode),
    });
    QUrl url(QLatin1StringView("https://sections.gog.com/v1/pages/%1/sections/%2").arg(pageHex, id));
    url.setQuery(parameters);
    QNetworkRequest request = api.createRequest();
    request.setUrl(url);
    return client.get(request);
}

QNetworkReply *api::GogApiClient::getStoreSections(const QString &pageHex, const QString &locale, const QString &countryCode, const QString &currencyCode)
{
    QUrlQuery parameters(
    {
        std::pair(QLatin1StringView("locale"), locale),
        std::pair(QLatin1StringView("countryCode"), countryCode),
        std::pair(QLatin1StringView("currencyCode"), currencyCode),
    });
    QUrl url(QLatin1StringView("https://sections.gog.com/v1/pages/%1").arg(pageHex));
    url.setQuery(parameters);
    QNetworkRequest request = api.createRequest();
    request.setUrl(url);
    return client.get(request);
}

QNetworkReply *api::GogApiClient::getStoreProductInfo(const QString &id, const QString &locale)
{
    QUrlQuery parameters(
    {
        std::pair(QLatin1StringView("expand"), QLatin1StringView("downloads,expanded_dlcs,related_products,changelog")),
        std::pair(QLatin1StringView("locale"), locale)
    });
    QUrl url(QLatin1StringView("https://api.gog.com/products/%1").arg(id));
    url.setQuery(parameters);
    QNetworkRequest request = api.createRequest();
    request.setUrl(url);
    return client.get(request);
}

QNetworkReply *api::GogApiClient::getUser(const QString &id)
{
    QNetworkRequest request = api.createRequest();
    request.setUrl(QUrl(QLatin1StringView("https://users.gog.com/users/%1").arg(id)));
    return client.get(request);
}

QNetworkReply *api::GogApiClient::getWishlist(const QString &query, const QString &order, unsigned short page)
{
    QUrlQuery parameters(
    {
        std::pair(QLatin1StringView("hiddenFlag"), QLatin1StringView("0")),
        std::pair(QLatin1StringView("mediaType"), QLatin1StringView("1")),
        std::pair(QLatin1StringView("sortBy"), order),
        std::pair(QLatin1StringView("page"), QString::number(page))
    });
    if (!query.isEmpty())
    {
        parameters.addQueryItem(QLatin1StringView("search"), query);
    }

    QUrl url(QLatin1StringView("https://embed.gog.com/account/wishlist/search"));
    url.setQuery(parameters);
    QNetworkRequest request = api.createRequest();
    request.setUrl(url);
    return client.get(request);
}

QNetworkReply *api::GogApiClient::getWishlistIds()
{
    QNetworkRequest request = api.createRequest();
    request.setUrl(QUrl(QLatin1StringView("https://embed.gog.com/user/wishlist.json")));
    return client.get(request);
}

QNetworkReply *api::GogApiClient::searchCatalog(const SortOrder &order,
                                                const CatalogFilter &filter,
                                                QString countryCode,
                                                QString locale,
                                                QString currencyCode,
                                                unsigned short page, unsigned int limit)
{
    QUrlQuery parameters;
    parameters.addQueryItem(QLatin1StringView("limit"), QString::number(limit));
    if (!filter.query.isEmpty())
    {
        parameters.addQueryItem(QLatin1StringView("query"), QLatin1StringView("like:") + filter.query);
    }
    if (filter.free)
    {
        parameters.addQueryItem(QLatin1StringView("price"), QLatin1StringView("between:0,0"));
    }
    if (!filter.developers.isEmpty())
    {
        parameters.addQueryItem(QLatin1StringView("developers"), QLatin1StringView("in:") + filter.developers.join(','));
    }
    if (!filter.publishers.isEmpty())
    {
        parameters.addQueryItem(QLatin1StringView("publishers"), QLatin1StringView("in:") + filter.publishers.join(','));
    }
    if (!filter.pageId.isEmpty())
    {
        parameters.addQueryItem(QLatin1StringView("pageId"), filter.pageId);
    }
    if (!filter.sectionId.isEmpty())
    {
        parameters.addQueryItem(QLatin1StringView("sectionId"), filter.sectionId);
    }
    if (!order.field.isEmpty())
    {
        parameters.addQueryItem(QLatin1StringView("order"), QLatin1StringView("%1:%2").arg(QLatin1StringView(order.ascending ? "asc" : "desc"), order.field));
    }
    if (!filter.genres.isEmpty())
    {
        parameters.addQueryItem(QLatin1StringView("genres"), QLatin1StringView("in:") + filter.genres.join(','));
    }
    if (!filter.excludeGenres.isEmpty())
    {
        parameters.addQueryItem(QLatin1StringView("excludeGenres"), QLatin1StringView("in:") + filter.excludeGenres.join(','));
    }
    if (!filter.languages.isEmpty())
    {
        parameters.addQueryItem(QLatin1StringView("languages"), QLatin1StringView("in:") + filter.languages.join(','));
    }
    if (!filter.systems.isEmpty())
    {
        parameters.addQueryItem(QLatin1StringView("systems"), QLatin1StringView("in:") + filter.systems.join(','));
    }
    if (!filter.tags.isEmpty() || filter.goodOldGames)
    {
        QStringList fullTagFilters(filter.tags);
        if (filter.goodOldGames)
        {
            fullTagFilters.append(QLatin1StringView("good-old-game"));
        }
        parameters.addQueryItem(QLatin1StringView("tags"), QLatin1StringView("is:") + fullTagFilters.join(','));
    }
    if (!filter.excludeTags.isEmpty())
    {
        parameters.addQueryItem(QLatin1StringView("excludeTags"), QLatin1StringView("in:") + filter.excludeTags.join(','));
    }
    if (!filter.features.isEmpty())
    {
        parameters.addQueryItem(QLatin1StringView("features"), QLatin1StringView("in:") + filter.features.join(','));
    }
    if (!filter.excludeFeatures.isEmpty())
    {
        parameters.addQueryItem(QLatin1StringView("excludeFeatures"), QLatin1StringView("in:") + filter.excludeFeatures.join(','));
    }
    if (!filter.releaseStatuses.isEmpty())
    {
        parameters.addQueryItem(QLatin1StringView("releaseStatuses"), QLatin1StringView("in:") + filter.releaseStatuses.join(','));
    }
    if (!filter.excludeReleaseStatuses.isEmpty())
    {
        parameters.addQueryItem(QLatin1StringView("excludeReleaseStatuses"), QLatin1StringView("in:") + filter.excludeReleaseStatuses.join(','));
    }
    if (!filter.productTypes.isEmpty())
    {
        parameters.addQueryItem(QLatin1StringView("productType"), QLatin1StringView("in:") + filter.productTypes.join(','));
    }
    if(filter.discounted)
    {
        parameters.addQueryItem(QLatin1StringView("discounted"), QLatin1StringView("eq:true"));
    }
    if(filter.hideOwned)
    {
        parameters.addQueryItem(QLatin1StringView("hideOwned"), QLatin1StringView("true"));
    }
    if(filter.onlyDlcForOwned)
    {
        parameters.addQueryItem(QLatin1StringView("onlyDlcForOwned"), QLatin1StringView("true"));
    }
    if (filter.onlyWishlisted)
    {
        parameters.addQueryItem(QLatin1StringView("wishlist"), QLatin1StringView("eq:true"));
    }
    parameters.addQueryItem(QLatin1StringView("page"), QString::number(page));
    parameters.addQueryItem(QLatin1StringView("countryCode"), countryCode);
    parameters.addQueryItem(QLatin1StringView("locale"), locale);
    parameters.addQueryItem(QLatin1StringView("currencyCode"), currencyCode);

    QUrl url(QLatin1StringView(filter.pageId.isEmpty() ? "https://catalog.gog.com/v1/catalog" : "https://catalog.gog.com/v1/filtered-catalog"));
    url.setQuery(parameters);
    QNetworkRequest request = api.createRequest();
    request.setUrl(url);
    return client.get(request);
}

QNetworkReply *api::GogApiClient::setWishlistVisibility(int visibility)
{
    QNetworkRequest request = api.createRequest();
    request.setUrl(QUrl(QLatin1StringView("https://embed.gog.com/account/save_sharing_wishlist/%1").arg(QString::number(visibility))));
    return client.get(request);
}

void api::GogApiClient::grant()
{
    oauth.grant();
}

void api::GogApiClient::logout()
{
    api.clearBearerToken();
    oauth.setToken(QString());
    oauth.setRefreshToken(QString());
    userId = QString();
    emit authenticated(false);
}
