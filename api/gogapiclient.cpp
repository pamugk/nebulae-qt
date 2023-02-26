#include "gogapiclient.h"

#include <QOAuthHttpServerReplyHandler>
#include <QProcessEnvironment>

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

bool api::GogApiClient::isAuthenticated()
{
    return !client.token().isNull();
}

QNetworkReply *api::GogApiClient::getAnything(const QString &url)
{
    return client.get(url);
}

QNetworkReply *api::GogApiClient::getOrdersHistory(const OrderFilter &filter, quint16 page)
{
    QVariantMap parameters;
    parameters["canceled"] = filter.cancelled ? "1" : "0";
    parameters["completed"] = filter.completed ? "1" : "0";
    parameters["in_progress"] = filter.inProgress ? "1" : "0";
    parameters["not_redeemed"] = filter.notRedeemed ? "1" : "0";
    parameters["pending"] = filter.pending ? "1" : "0";
    parameters["redeemed"] = filter.redeemed ? "1" : "0";
    parameters["page"] = QString::number(page);
    if (!filter.query.isNull() && !filter.query.isEmpty())
    {
        parameters["search"] = filter.query;
    }
    return client.get(QUrl("https://embed.gog.com/account/settings/orders/data"), parameters);
}

QNetworkReply *api::GogApiClient::getWishlist(const QString &query, const QString &order, quint16 page)
{
    QVariantMap parameters;
    parameters["hiddenFlag"] = "0";
    parameters["mediaType"] = "1";
    parameters["sortBy"] = order;
    parameters["page"] = QString::number(page);
    if (!query.isNull() && !query.isEmpty())
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
                                           quint16 page, quint32 limit)
{
    QVariantMap parameters;
    parameters["limit"] = QString::number(limit);
    if (!filter.query.isNull() && !filter.query.isEmpty())
    {
        parameters["query"] = "like:" + filter.query;
    }
    parameters["order"] = QString("%1:%2").arg(order.ascending ? "asc" : "desc", order.field);
    if (!filter.genres.isEmpty())
    {
        parameters["genres"] = "in:" + filter.genres.join(',');
    }
    if (!filter.excludeGenres.isEmpty())
    {
        parameters["excludeGenres"] = "in:" + filter.excludeGenres.join(',');
    }
    if (!filter.languages.isEmpty())
    {
        parameters["languages"] = "in:" + filter.languages.join(',');
    }
    if (!filter.systems.isEmpty())
    {
        parameters["systems"] = "in:" + filter.systems.join(',');
    }
    if (!filter.tags.isEmpty())
    {
        parameters["tags"] = "in:" + filter.tags.join(',');
    }
    if (!filter.excludeTags.isEmpty())
    {
        parameters["excludeTags"] = "in:" + filter.excludeTags.join(',');
    }
    if (!filter.features.isEmpty())
    {
        parameters["features"] = "in:" + filter.features.join(',');
    }
    if (!filter.excludeFeatures.isEmpty())
    {
        parameters["excludeFeatures"] = "in:" + filter.excludeFeatures.join(',');
    }
    if (!filter.releaseStatuses.isEmpty())
    {
        parameters["releaseStatuses"] = "in:" + filter.releaseStatuses.join(',');
    }
    if (!filter.excludeReleaseStatuses.isEmpty())
    {
        parameters["excludeReleaseStatuses"] = "in:" + filter.excludeReleaseStatuses.join(',');
    }
    if (!filter.productTypes.isEmpty())
    {
        parameters["productType"] = "in:" + filter.productTypes.join(',');
    }
    if(filter.discounted)
    {
        parameters["discounted"] = "eq:true";
    }
    if(filter.hideOwned)
    {
        parameters["hideOwned"] = "true";
    }
    parameters["page"] = QString::number(page);
    parameters["countryCode"] = countryCode;
    parameters["locale"] = locale;
    parameters["currencyCode"] = currencyCode;
    return client.get(QUrl("https://catalog.gog.com/v1/catalog"), parameters);
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
