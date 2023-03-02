#ifndef GOGAPICLIENT_H
#define GOGAPICLIENT_H

#include <QtGlobal>

#include <QObject>
#include <QOAuth2AuthorizationCodeFlow>
#include <QSettings>

#include "./models/catalogfilter.h"
#include "./models/orderfilter.h"
#include "./models/sort.h"

namespace api
{
    class GogApiClient : public QObject
    {
        Q_OBJECT

    public:
        explicit GogApiClient(QObject *parent = nullptr);
        bool isAuthenticated();

        QNetworkReply *getAnything(const QString &url);
        QNetworkReply *getCatalogProductInfo(quint64 id, const QString &locale);
        QNetworkReply *getOrdersHistory(const OrderFilter &filter, quint16 page = 1);
        QNetworkReply *getProductAverageRating(quint64 productId, const QString &reviewer = QString());
        QNetworkReply *getProductPrices(quint64 productId, const QString &countryCode);
        QNetworkReply *getProductReviews(quint64 productId,
                                         const QStringList &languages,
                                         const SortOrder &order,
                                         quint32 limit, quint32 page);
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
