#include "gogapiclient.h"

#include <QOAuthHttpServerReplyHandler>
#include <QProcessEnvironment>

GogApiClient::GogApiClient(QObject *parent)
    : QObject{parent}
{
    auto environment = QProcessEnvironment::systemEnvironment();

    client.setReplyHandler(new QOAuthHttpServerReplyHandler(6543, &client));
    client.setClientIdentifier(environment.value("CLIENT_ID"));
    client.setClientIdentifierSharedKey(environment.value("CLIENT_SECRET"));
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

QNetworkReply *GogApiClient::getWishlist(const QString query, WishlistSortOrder order, quint16 page)
{
    QVariantMap parameters;
    if (!query.isNull() && !query.isEmpty())
    {
        parameters["search"] = query;
    }
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
    return client.get(QUrl("https://embed.gog.com/account/wishlist/search"), parameters);
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
