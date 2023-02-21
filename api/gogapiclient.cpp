#include "gogapiclient.h"

#include <QOAuthHttpServerReplyHandler>
#include <QProcessEnvironment>

GogApiClient::GogApiClient(QObject *parent)
    : QObject{parent}
{
    auto environment = QProcessEnvironment::systemEnvironment();
    client = new QOAuth2AuthorizationCodeFlow(this);

    client->setReplyHandler(new QOAuthHttpServerReplyHandler(6543, client));
    client->setClientIdentifier(environment.value("CLIENT_ID"));
    client->setClientIdentifierSharedKey(environment.value("CLIENT_SECRET"));
    client->setAuthorizationUrl(QUrl("https://auth.gog.com/auth"));
    client->setAccessTokenUrl(QUrl("https://auth.gog.com/token"));
    client->setModifyParametersFunction([&](QAbstractOAuth::Stage stage, QVariantMap *parameters) {
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

    connect(client, &QOAuth2AuthorizationCodeFlow::statusChanged, [=](
                QAbstractOAuth::Status status) {
            if (status == QAbstractOAuth::Status::Granted) {
                emit authenticated();
            }
        });
    connect(client, &QOAuth2AuthorizationCodeFlow::authorizeWithBrowser, this, &GogApiClient::authorize);
}

void GogApiClient::grant()
{
    client->grant();
}
