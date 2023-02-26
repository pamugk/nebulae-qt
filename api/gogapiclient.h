#ifndef GOGAPICLIENT_H
#define GOGAPICLIENT_H

#include <QObject>
#include <QOAuth2AuthorizationCodeFlow>
#include <QSettings>

#include "./models/catalogfilter.h"
#include "./models/orderfilter.h"
#include "./models/sort.h"
#include "./models/wishlist.h"

namespace api
{
    class GogApiClient : public QObject
    {
        Q_OBJECT

    public:
        explicit GogApiClient(QObject *parent = nullptr);
        bool isAuthenticated();

        QNetworkReply *getAnything(const QString &url);
        QNetworkReply *getOrdersHistory(const OrderFilter &filter, quint16 page = 1);
        QNetworkReply *getWishlist(const QString &query = QString(),
                                   const QString &order = "title",
                                   quint16 page = 1);
        QNetworkReply *searchCatalog(const SortOrder &order,
                                     const CatalogFilter &filter,
                                     QString countryCode,
                                     QString locale,
                                     QString currencyCode,
                                     quint16 page = 1, quint32 limit = 48);

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
}

#endif // GOGAPICLIENT_H
