#ifndef GOGAPICLIENT_H
#define GOGAPICLIENT_H

#include <QNetworkAccessManager>
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

        QNetworkReply *getAchievements();
        QNetworkReply *getAnything(const QString &url);
        QNetworkReply *getCatalogProductInfo(unsigned long long id, const QString &locale);
        QNetworkReply *getOrdersHistory(const OrderFilter &filter, unsigned short page = 1);
        QNetworkReply *getOwnedProductInfo(unsigned long long id, const QString &locale);
        QNetworkReply *getOwnedProducts(const QString &query = QString(),
                                   const QString &order = "title",
                                   unsigned short page = 1);
        QNetworkReply *getPlayTime();
        QNetworkReply *getProductAchievements(unsigned long long productId);
        QNetworkReply *getProductAverageRating(unsigned long long productId, const QString &reviewer = QString());
        QNetworkReply *getProductPrices(unsigned long long productId, const QString &countryCode);
        QNetworkReply *getProductRecommendationsPurchasedTogether(unsigned long long productId,
                                                                 const QString &countryCode,
                                                                 const QString &currency,
                                                                 unsigned char limit = 8);
        QNetworkReply *getProductRecommendationsSimilar(unsigned long long productId,
                                                       const QString &countryCode,
                                                       const QString &currency,
                                                       unsigned char limit = 8);
        QNetworkReply *getProductReviews(unsigned long long productId,
                                         const QStringList &languages,
                                         const SortOrder &order,
                                         unsigned short limit, unsigned short page);
        QNetworkReply *getWishlist(const QString &query = QString(),
                                   const QString &order = "title",
                                   unsigned short page = 1);
        QNetworkReply *searchCatalog(const SortOrder &order,
                                     const CatalogFilter &filter,
                                     QString countryCode,
                                     QString locale,
                                     QString currencyCode,
                                     unsigned short page = 1, unsigned int limit = 48);

    public slots:
        void grant();
        void setStoreCredentials(bool value);

    signals:
        void authorize(const QUrl &authUrl);
        void authenticated(bool authenticated);

    private:
        QNetworkAccessManager client_tmp;
        QOAuth2AuthorizationCodeFlow client;
        QSettings settings;
        bool storeTokens;
    };
}

#endif // GOGAPICLIENT_H
