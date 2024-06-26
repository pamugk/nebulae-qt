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

        QString currentUserId() const;
        QNetworkReply *getAchievements();
        QNetworkReply *getAnything(const QString &url);
        QNetworkReply *getCatalogProductInfo(const QString &id, const QString &locale);
        QNetworkReply *getCurrentUser();
        QNetworkReply *getCurrentUserGameTimeStatistics();
        QNetworkReply *getCurrentUserPlatformAchievements(const QString &platform, const QString &pageToken);
        QNetworkReply *getCurrentUserPlatformReleaseAchievements(const QString &platformId, const QString &platformReleaseId, const QString &pageToken);
        QNetworkReply *getCurrentUserPlatformReleaseGameTimeStatistics(const QString &platformId, const QString &platformReleaseId);
        QNetworkReply *getCurrentUserReleases();
        QNetworkReply *getGame(const QString &id);
        QNetworkReply *getNews(unsigned short pageToken, const QString &locale,
                               unsigned char limit);
        QNetworkReply *getNowOnSale(const QString &locale, const QString &countryCode, const QString &currencyCode);
        QNetworkReply *getNowOnSaleSection(const QString &sectionId);
        QNetworkReply *getOrdersHistory(const OrderFilter &filter, unsigned short page = 1);
        QNetworkReply *getOwnedLicensesIds();
        QNetworkReply *getOwnedProducts(const QString &query = QString(),
                                        const QString &order = "title",
                                        unsigned short page = 1);
        QNetworkReply *getPlatformRelease(const QString &platformId, const QString &platformReleaseId);
        QNetworkReply *getPlatformReleaseAchievements(const QString &platformId, const QString &platformReleaseId,
                                                      const QString &locale);
        QNetworkReply *getProductAchievements(const QString &productId);
        QNetworkReply *getProductAverageRating(const QString &productId, const QString &reviewer = QString());
        QNetworkReply *getProductPrices(const QString &productId, const QString &countryCode);
        QNetworkReply *getProductRecommendationsPurchasedTogether(const QString &productId,
                                                                 const QString &countryCode,
                                                                 const QString &currency,
                                                                 unsigned char limit = 8);
        QNetworkReply *getProductRecommendationsSimilar(const QString &productId,
                                                       const QString &countryCode,
                                                       const QString &currency,
                                                       unsigned char limit = 8);
        QNetworkReply *getProductReviews(const QString &productId,
                                         const ReviewFilters &filters,
                                         const SortOrder &order,
                                         unsigned short limit, unsigned short page);
        QNetworkReply *getRecommendedDlcs();
        QNetworkReply *getRelease(const QString &id);
        QNetworkReply *getSeriesGames(unsigned long long seriesId);
        QNetworkReply *getSeriesPrices(unsigned long long seriesId,
                                       const QString &countryCode,
                                       const QString &currencyCode);
        QNetworkReply *getStoreCustomSection(const QString &id);
        QNetworkReply *getStoreDiscoverGamesForYou();
        QNetworkReply *getStoreDiscoverNewGames();
        QNetworkReply *getStoreDiscoverUpcomingGames();
        QNetworkReply *getStoreProductInfo(const QString &id, const QString &locale);
        QNetworkReply *getUser(const QString &id);
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
        QString userId;
    };
}

#endif // GOGAPICLIENT_H
