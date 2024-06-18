#ifndef GOGAPICLIENT_H
#define GOGAPICLIENT_H

#include <QOAuth2AuthorizationCodeFlow>
#include <QObject>

#include "authdatastorage.h"
#include "./models/catalogfilter.h"
#include "./models/orderfilter.h"
#include "./models/reviewfilters.h"
#include "./models/sort.h"

namespace api
{
    class GogApiClient : public QObject
    {
        Q_OBJECT

    public:
        explicit GogApiClient(AuthDataStorage *tokenStorage, QObject *parent = nullptr);
        bool isAuthenticated();

        QNetworkReply *getAchievements();
        QNetworkReply *getAnything(const QString &url);
        QNetworkReply *getCatalogProductInfo(unsigned long long id, const QString &locale);
        QNetworkReply *getCurrentUser();
        QNetworkReply *getNews(unsigned short pageToken, const QString &locale,
                               unsigned char limit);
        QNetworkReply *getNowOnSale(const QString &locale, const QString &countryCode, const QString &currencyCode);
        QNetworkReply *getNowOnSaleSection(const QString &sectionId);
        QNetworkReply *getOrdersHistory(const OrderFilter &filter, unsigned short page = 1);
        QNetworkReply *getOwnedLicensesIds();
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
                                         const ReviewFilters &filters,
                                         const SortOrder &order,
                                         unsigned short limit, unsigned short page);
        QNetworkReply *getRecommendedDlcs();
        QNetworkReply *getSeriesGames(unsigned long long seriesId);
        QNetworkReply *getSeriesPrices(unsigned long long seriesId,
                                       const QString &countryCode,
                                       const QString &currencyCode);
        QNetworkReply *getStoreCustomSection(const QString &id);
        QNetworkReply *getStoreDiscoverGamesForYou();
        QNetworkReply *getStoreDiscoverNewGames();
        QNetworkReply *getStoreDiscoverUpcomingGames();
        QNetworkReply *getUser(unsigned long long id);
        QNetworkReply *getWishlist(const QString &query = QString(),
                                   const QString &order = "title",
                                   unsigned short page = 1);
        QNetworkReply *getWishlistIds();
        QNetworkReply *searchCatalog(const SortOrder &order,
                                     const CatalogFilter &filter,
                                     QString countryCode,
                                     QString locale,
                                     QString currencyCode,
                                     unsigned short page = 1, unsigned int limit = 48);
        QNetworkReply *setWishlistVisibility(int visibility);

    public slots:
        void grant();
        void logout();

    signals:
        void authorize(const QUrl &authUrl);
        void authenticated(bool authenticated);

    private:
        QOAuth2AuthorizationCodeFlow client;
        bool refreshingToken;
        std::optional<unsigned long long> userId;
    };
}

#endif // GOGAPICLIENT_H
