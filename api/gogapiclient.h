#ifndef GOGAPICLIENT_H
#define GOGAPICLIENT_H

#include <QObject>
#include <QOAuth2AuthorizationCodeFlow>

class GogApiClient : public QObject
{
    Q_OBJECT

public:
    explicit GogApiClient(QObject *parent = nullptr);

public slots:
    void grant();

signals:
    void authorize(const QUrl &authUrl);
    void authenticated();

private:
    QOAuth2AuthorizationCodeFlow *client;
};

#endif // GOGAPICLIENT_H
