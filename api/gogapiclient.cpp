#include "gogapiclient.h"

#include <QNetworkRequest>
#include <QOAuthHttpServerReplyHandler>
#include <QProcessEnvironment>
#include <QUrlQuery>

api::GogApiClient::GogApiClient(QObject *parent)
    : QObject{parent}
{
    auto environment = QProcessEnvironment::systemEnvironment();

    client.setReplyHandler(new QOAuthHttpServerReplyHandler(6543, &client));
    client.setClientIdentifier(environment.value("GOG_CLIENT_ID"));
    client.setClientIdentifierSharedKey(environment.value("GOG_CLIENT_SECRET"));
    client.setAuthorizationUrl(QUrl("https://auth.gog.com/auth"));
    client.setAccessTokenUrl(QUrl("https://auth.gog.com/token"));
    client.setModifyParametersFunction([&](QAbstractOAuth::Stage stage, QMultiMap<QString, QVariant> *parameters) {
                                            switch (stage)
                                            {
                                            case QAbstractOAuth::Stage::RequestingTemporaryCredentials:
                                            case QAbstractOAuth::Stage::RequestingAuthorization:
                                            {
                                                parameters->insert("layout", "client2");
                                            }
                                            case QAbstractOAuth::Stage::RequestingAccessToken:
                                            {
                                                parameters->remove("redirect_uri");
                                                parameters->insert("redirect_uri", "https://embed.gog.com/on_login_success?origin=client");
                                                break;
                                            }
                                            case QAbstractOAuth::Stage::RefreshingAccessToken:
                                                break;
                                            }
                                        });

    connect(&client, &QOAuth2AuthorizationCodeFlow::statusChanged, this, [this](
                QAbstractOAuth::Status status) {
            if (status == QAbstractOAuth::Status::Granted) {
                emit authenticated(true);
            }
        });
    connect(&client, &QOAuth2AuthorizationCodeFlow::authorizeWithBrowser, this, &GogApiClient::authorize);

    if (settings.contains("/credentials/refresh_token"))
    {
        client.setRefreshToken(settings.value("/credentials/refresh_token").toString());
        client.setToken(settings.value("/credentials/token", QString()).toString());
    }

    connect(&client, &QOAuth2AuthorizationCodeFlow::tokenChanged, this, [this](const QString & newToken){
        if (storeTokens)
        {
            if (newToken.isEmpty())
            {
                settings.remove("/credentials/token");
            }
            else
            {
                settings.setValue("/credentials/token", newToken);
            }
        }
    });

    connect(&client, &QOAuth2AuthorizationCodeFlow::refreshTokenChanged, this, [this](const QString & newToken){
        if (storeTokens)
        {
            if (newToken.isEmpty())
            {
                settings.remove("/credentials/refresh_token");
            }
            else
            {
                settings.setValue("/credentials/refresh_token", newToken);
            }
        }
    });
}

bool api::GogApiClient::isAuthenticated()
{
    return !client.token().isNull();
}

QNetworkReply *api::GogApiClient::getAchievements()
{
    return client.get(QUrl(QString("https://gameplay.gog.com/users/%1/sessions").arg("")));
}

QNetworkReply *api::GogApiClient::getAnything(const QString &url)
{
    return client.get(url);
}

QNetworkReply *api::GogApiClient::getCatalogProductInfo(unsigned long long id, const QString &locale)
{
    QVariantMap parameters;
    parameters["locale"] = locale;
    return client.get(QUrl("https://api.gog.com/v2/games/" + QString::number(id)), parameters);
}

QNetworkReply *api::GogApiClient::getNews(unsigned short pageToken, const QString &locale,
                                          unsigned char limit)
{
    QUrl url("https://api.gog.com/news");
    url.setQuery(QUrlQuery({
                               std::pair("language_code", locale),
                               std::pair("page_token", QString::number(pageToken)),
                               std::pair("limit", QString::number(limit)),
                           }));
    return client.get(url);
}

QNetworkReply *api::GogApiClient::getOrdersHistory(const OrderFilter &filter, unsigned short page)
{
    QVariantMap parameters;
    parameters["canceled"] = filter.cancelled ? "1" : "0";
    parameters["completed"] = filter.completed ? "1" : "0";
    parameters["in_progress"] = filter.inProgress ? "1" : "0";
    parameters["not_redeemed"] = filter.notRedeemed ? "1" : "0";
    parameters["pending"] = filter.pending ? "1" : "0";
    parameters["redeemed"] = filter.redeemed ? "1" : "0";
    parameters["page"] = QString::number(page);
    if (!filter.query.isEmpty())
    {
        parameters["search"] = filter.query;
    }
    return client.get(QUrl("https://embed.gog.com/account/settings/orders/data"), parameters);
}

QNetworkReply *api::GogApiClient::getOwnedProductInfo(unsigned long long id, const QString &locale)
{
    QVariantMap parameters;
    parameters["expand"] = "downloads,expanded_dlcs,description,screenshots,videos,related_products,changelog";
    parameters["locale"] = locale;
    return client.get(QUrl("https://api.gog.com/products/" + QString::number(id)), parameters);
}

QNetworkReply *api::GogApiClient::getOwnedProducts(const QString &query, const QString &order, unsigned short page)
{
    QVariantMap parameters;
    parameters["hiddenFlag"] = "0";
    parameters["mediaType"] = "1";
    parameters["sortBy"] = order;
    parameters["page"] = QString::number(page);
    if (!query.isEmpty())
    {
        parameters["search"] = query;
    }
    return client.get(QUrl("https://embed.gog.com/account/getFilteredProducts"), parameters);
}

QNetworkReply *api::GogApiClient::getPlayTime()
{
    return client.get(QUrl(QString("https://gameplay.gog.com/users/%1/sessions").arg("")));
}

QNetworkReply *api::GogApiClient::getProductAchievements(unsigned long long productId)
{
    return client.get(QUrl(QString("https://gameplay.gog.com/clients/%1/users/%2/sessions").arg(QString::number(productId), "")));
}

QNetworkReply *api::GogApiClient::getProductAverageRating(unsigned long long productId, const QString &reviewer)
{
    QVariantMap parameters;
    if (!reviewer.isEmpty())
    {
        parameters["reviewer"] = reviewer;
    }
    return client.get(QUrl(QString("https://reviews.gog.com/v1/products/%1/averageRating").arg(QString::number(productId))), parameters);
}

QNetworkReply *api::GogApiClient::getProductPrices(unsigned long long productId, const QString &countryCode)
{
    QVariantMap parameters;
    parameters["countryCode"] = countryCode;
    return client.get(QUrl(QString("https://api.gog.com/products/%1/prices").arg(QString::number(productId))), parameters);
}

QNetworkReply *api::GogApiClient::getProductRecommendationsPurchasedTogether(unsigned long long productId,
                                                                            const QString &countryCode,
                                                                            const QString &currency,
                                                                            unsigned char limit)
{
    QUrl url(QString("https://recommendations-api.gog.com/v1/recommendations/purchased_together/%1").arg(QString::number(productId)));
    url.setQuery(QUrlQuery({
                               std::pair("country_code", countryCode),
                               std::pair("currency", currency),
                               std::pair("limit", QString::number(limit)),
                           }));
    return client.get(url);
}

QNetworkReply *api::GogApiClient::getProductRecommendationsSimilar(unsigned long long productId,
                                                                  const QString &countryCode,
                                                                  const QString &currency,
                                                                  unsigned char limit)
{
    QUrl url(QString("https://recommendations-api.gog.com/v1/recommendations/similar/%1").arg(QString::number(productId)));
    url.setQuery(QUrlQuery({
                               std::pair("country_code", countryCode),
                               std::pair("currency", currency),
                               std::pair("limit", QString::number(limit)),
                           }));
    return client.get(url);
}

QNetworkReply *api::GogApiClient::getProductReviews(unsigned long long productId,
                                                    const QStringList &languages,
                                                    const SortOrder &order,
                                                    unsigned short limit, unsigned short page)
{
    QVariantMap parameters;
    if (!languages.isEmpty())
    {
        parameters["language"] = "in:" + languages.join(',');
    }
    parameters["limit"] = QString::number(limit);
    parameters["order"] = QString("%1:%2").arg(order.ascending ? "asc" : "desc", order.field);
    parameters["page"] = QString::number(page);
    return client.get(QUrl(QString("https://reviews.gog.com/v1/products/%1/reviews").arg(QString::number(productId))), parameters);
}

QNetworkReply *api::GogApiClient::getSeriesGames(unsigned long long seriesId)
{
    QUrl url("https://api.gog.com/v2/games");
    url.setQuery(QUrlQuery({
                               std::pair("seriesId", QString::number(seriesId)),
                           }));
    return client.get(url);
}

QNetworkReply *api::GogApiClient::getSeriesPrices(unsigned long long seriesId,
                                                  const QString &countryCode,
                                                  const QString &currencyCode)
{
    QUrl url("https://api.gog.com/products/prices");
    url.setQuery(QUrlQuery({
                               std::pair("seriesId", QString::number(seriesId)),
                               std::pair("countryCode", countryCode),
                               std::pair("currency", currencyCode),
                           }));
    return client.get(url);
}

QNetworkReply *api::GogApiClient::getStoreCustomSection(const QString &id)
{
    return client.get(QUrl(QString("https://api.gog.com/custom_sections/%1").arg(id)));
}

QNetworkReply *api::GogApiClient::getStoreDiscoverNewGames()
{
    return client.get(QUrl("https://embed.gog.com/discover_games/new"));
}

QNetworkReply *api::GogApiClient::getStoreDiscoverUpcomingGames()
{
    return client.get(QUrl("https://embed.gog.com/discover_games/upcoming"));
}

QNetworkReply *api::GogApiClient::getWishlist(const QString &query, const QString &order, unsigned short page)
{
    QVariantMap parameters;
    parameters["hiddenFlag"] = "0";
    parameters["mediaType"] = "1";
    parameters["sortBy"] = order;
    parameters["page"] = QString::number(page);
    if (!query.isEmpty())
    {
        parameters["search"] = query;
    }
    return client.get(QUrl("https://embed.gog.com/account/wishlist/search"), parameters);
}

QNetworkReply *api::GogApiClient::searchCatalog(const SortOrder &order,
                                                const CatalogFilter &filter,
                                                QString countryCode,
                                                QString locale,
                                                QString currencyCode,
                                                unsigned short page, unsigned int limit)
{
    QUrlQuery query;
    query.addQueryItem("limit", QString::number(limit));
    if (!filter.query.isEmpty())
    {
        query.addQueryItem("query", "like:" + filter.query);
    }
    query.addQueryItem("order", QString("%1:%2").arg(order.ascending ? "asc" : "desc", order.field));
    if (!filter.genres.isEmpty())
    {
        query.addQueryItem("genres", filter.genres.join(','));
    }
    if (!filter.excludeGenres.isEmpty())
    {
        query.addQueryItem("excludeGenres", "in:" + filter.excludeGenres.join(','));
    }
    if (!filter.languages.isEmpty())
    {
        query.addQueryItem("languages", "in:" + filter.languages.join(','));
    }
    if (!filter.systems.isEmpty())
    {
        query.addQueryItem("systems", "in:" + filter.systems.join(','));
    }
    if (!filter.tags.isEmpty())
    {
        query.addQueryItem("tags", "in:" + filter.tags.join(','));
    }
    if (!filter.excludeTags.isEmpty())
    {
        query.addQueryItem("excludeTags", "in:" + filter.excludeTags.join(','));
    }
    if (!filter.features.isEmpty())
    {
        query.addQueryItem("features", "in:" + filter.features.join(','));
    }
    if (!filter.excludeFeatures.isEmpty())
    {
        query.addQueryItem("excludeFeatures", "in:" + filter.excludeFeatures.join(','));
    }
    if (!filter.releaseStatuses.isEmpty())
    {
        query.addQueryItem("releaseStatuses", "in:" + filter.releaseStatuses.join(','));
    }
    if (!filter.excludeReleaseStatuses.isEmpty())
    {
        query.addQueryItem("excludeReleaseStatuses", "in:" + filter.excludeReleaseStatuses.join(','));
    }
    if (!filter.productTypes.isEmpty())
    {
        query.addQueryItem("productType", "in:" + filter.productTypes.join(','));
    }
    if(filter.discounted)
    {
        query.addQueryItem("discounted", "eq:true");
    }
    if(filter.hideOwned)
    {
        query.addQueryItem("hideOwned", "true");
    }
    query.addQueryItem("page", QString::number(page));
    query.addQueryItem("countryCode", countryCode);
    query.addQueryItem("locale", locale);
    query.addQueryItem("currencyCode", currencyCode);

    QUrl url("https://catalog.gog.com/v1/catalog");
    url.setQuery(query);
    return client_tmp.get(QNetworkRequest(url));
}

void api::GogApiClient::grant()
{
    client.grant();
}

void api::GogApiClient::setStoreCredentials(bool value)
{
    storeTokens = value;
    if (storeTokens)
    {
        settings.setValue("/credentials/refresh_token", client.refreshToken());
        settings.setValue("/credentials/token", client.token());
    }
    else
    {
        settings.remove("/credentials/refresh_token");
        settings.remove("/credentials/token");
    }
}
