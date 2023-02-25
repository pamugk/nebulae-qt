#ifndef GOGAPICLIENT_H
#define GOGAPICLIENT_H

#include <QObject>
#include <QOAuth2AuthorizationCodeFlow>
#include <QSettings>

#include "./models/wishlist.h"

class GogApiClient : public QObject
{
    Q_OBJECT

public:
    explicit GogApiClient(QObject *parent = nullptr);
    bool isAuthenticated();

    QNetworkReply *getAnything(const QString &url);
    QNetworkReply *getOrdersHistory(bool cancelled, bool completed, bool inProgress,
                                    bool notRedeemed, bool pending,  bool redeemed,
                                    const QString &query, quint16 page = 1);
    QNetworkReply *getWishlist(const QString query = QString(),
                               WishlistSortOrder order = WishlistSortOrder::TITLE,
                               quint16 page = 1);

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
