#include "gogapiclient.h"

#include <QOAuthHttpServerReplyHandler>
#include <QProcessEnvironment>

GogApiClient::GogApiClient(QObject *parent)
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
                emit authenticated();
            }
        });
    connect(&client, &QOAuth2AuthorizationCodeFlow::authorizeWithBrowser, this, &GogApiClient::authorize);

    client.setRefreshToken(settings.value("/credentials/refresh_token", QString()).toString());
    client.setToken(settings.value("/credentials/token", QString()).toString());

    connect(&client, &QOAuth2AuthorizationCodeFlow::tokenChanged, this, [this](const QString & newToken){
        if (storeTokens)
        {
            settings.setValue("/credentials/token", newToken);
        }
    });

    connect(&client, &QOAuth2AuthorizationCodeFlow::refreshTokenChanged, this, [this](const QString & newToken){
        if (storeTokens)
        {
            settings.setValue("/credentials/refresh_token", newToken);
        }
    });
}

bool GogApiClient::isAuthenticated()
{
    return !client.token().isNull();
}

QNetworkReply *GogApiClient::getAnything(const QString &url)
{
    return client.get(url);
}

QNetworkReply *GogApiClient::getOrdersHistory(
        bool cancelled, bool completed, bool inProgress,
        bool notRedeemed, bool pending, bool redeemed,
        const QString &query, quint16 page)
{
    QVariantMap parameters;
    parameters["canceled"] = cancelled ? "1" : "0";
    parameters["completed"] = completed ? "1" : "0";
    parameters["in_progress"] = inProgress ? "1" : "0";
    parameters["not_redeemed"] = notRedeemed ? "1" : "0";
    parameters["pending"] = pending ? "1" : "0";
    parameters["redeemed"] = redeemed ? "1" : "0";
    parameters["page"] = QString::number(page);
    if (!query.isEmpty())
    {
        parameters["search"] = query;
    }
    return client.get(QUrl("https://embed.gog.com/account/settings/orders/data"), parameters);
}

QNetworkReply *GogApiClient::getWishlist(const QString query, WishlistSortOrder order, quint16 page)
{
    QVariantMap parameters;
    parameters["hiddenFlag"] = "0";
    parameters["mediaType"] = "1";
    QString orderString;
    switch (order)
    {
    case WishlistSortOrder::DATE_ADDED:
        orderString = "date_added";
        break;
    case WishlistSortOrder::TITLE:
        orderString = "title";
        break;
    case WishlistSortOrder::USER_REVIEWS:
        orderString = "user_reviews";
        break;
    }
    parameters["sortBy"] = orderString;
    parameters["page"] = QString::number(page);
    if (!query.isNull() && !query.isEmpty())
    {
        parameters["search"] = query;
    }
    return client.get(QUrl("https://embed.gog.com/account/wishlist/search"), parameters);
}

QNetworkReply *GogApiClient::searchCatalog(const QString orderField, bool orderAscending,
                                           const QString &countryCode, const QString &locale, const QString &currencyCode,
                                           const QString &query, bool discounted,
                                           const QStringList &genres, const QStringList &languages,
                                           const QStringList &systems, const QStringList &tags,
                                           const QStringList &features, const QStringList &releaseStatuses,
                                           const QStringList &productTypes,
                                           quint16 page, quint32 limit)
{
    QVariantMap parameters;
    parameters["limit"] = QString::number(limit);
    if (!query.isNull() && !query.isEmpty())
    {
        parameters["query"] = "like:" + query;
    }
    parameters["order"] = QString("%1:%2").arg(orderAscending ? "asc" : "desc", orderField);
    if (!productTypes.isEmpty())
    {
        parameters["genre"] = "in:" + genres.join(',');
    }
    if (!productTypes.isEmpty())
    {
        parameters["language"] = "in:" + languages.join(',');
    }
    if (!productTypes.isEmpty())
    {
        parameters["system"] = "in:" + systems.join(',');
    }
    if (!productTypes.isEmpty())
    {
        parameters["tag"] = "in:" + tags.join(',');
    }
    if (!productTypes.isEmpty())
    {
        parameters["feature"] = "in:" + features.join(',');
    }
    if (!productTypes.isEmpty())
    {
        parameters["releaseStatus"] = "in:" + releaseStatuses.join(',');
    }
    if (!productTypes.isEmpty())
    {
        parameters["productType"] = "in:" + productTypes.join(',');
    }
    parameters["page"] = QString::number(page);
    parameters["countryCode"] = countryCode;
    parameters["locale"] = locale;
    parameters["currencyCode"] = currencyCode;
    return client.get(QUrl("https://catalog.gog.com/v1/catalog"), parameters);
}

void GogApiClient::grant()
{
    client.grant();
}

void GogApiClient::setStoreCredentials(bool value)
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
