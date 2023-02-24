#ifndef GOGAPICLIENT_H
#define GOGAPICLIENT_H

#include <QObject>
#include <QOAuth2AuthorizationCodeFlow>
#include <QSettings>

class GogApiClient : public QObject
{
    Q_OBJECT

public:
    explicit GogApiClient(QObject *parent = nullptr);
    bool isAuthenticated();

public slots:
    void grant();
    void setStoreCredentials(bool value);

signals:
    void authorize(const QUrl &authUrl);
    void authenticated();

private:
    QOAuth2AuthorizationCodeFlow client;
    QSettings settings;
    bool storeTokens;
};

#endif // GOGAPICLIENT_H
