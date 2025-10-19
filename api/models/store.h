#ifndef STORE_H
#define STORE_H

#include <QDateTime>
#include <QString>
#include <QVector>

#include "catalog.h"
#include "news.h"

namespace api
{
    struct StoreBannerItem
    {
        QString id;
        std::optional<CatalogProduct> product;
        QString background;
        QString logo;
        QString gradientBaseColor;
        QString title;
        QString subtitle;
        QDateTime visibleFrom;
        QDateTime visibleTo;
        struct
        {
            QString url;
            QString buttonText;
            QString discountText;
        } customProperties;
        bool useDarkColorFont;
    };

    struct StoreDiscoverColumn
    {
        QVector<CatalogProduct> items;
        QString title;
        QString seeMoreLink;
    };

    struct StoreNowOnSaleTabCard
    {
        QString background;
        QString logo;
        QString color;
        std::array<unsigned char, 3> colorRgbArray;
        QString text;
        QString textSlug;
        unsigned char discountValue;
        bool discountUpTo;
        QString url;
        QDateTime countdownDate;
    };

    struct StoreNowOnSaleTab
    {
        QString id;
        QString title;
        StoreNowOnSaleTabCard bigThingy;
    };

    struct StoreProductPrice
    {
        double baseAmount;
        double finalAmount;
        unsigned char discountPercentage;
        bool free;
    };

    struct StoreProduct
    {
        QString id;
        QString title;
        QString image;
        StoreProductPrice price;
        QVector<QString> supportedOperatingSystems;

        bool comingSoon;
        bool inDevelopment;
        bool availableForSale;
        bool buyable;
        bool movie;
        bool visibleInCatalog;
        bool preorder;
    };

    struct StoreSection
    {
        QString id;
        QString sectionType;
        bool personalized;
        bool hideOnLoad;
        bool loadOnEmbed;
        QString contentSourceType;
    };

    struct StoreVerticalBannerItem
    {
        QString promoId;
        QString title;
        QString url;
        QString backgroundImage;
        QString color;
        QString discount;
        bool discountUpTo;
        QDateTime promoEndDate;
    };

    struct GetStoreAnnouncementSectionResponse
    {
        StoreBannerItem data;
    };

    struct GetStoreDiscoverGamesSectionResponse
    {
        StoreDiscoverColumn columnLeft;
        StoreDiscoverColumn columnRight;
    };

    struct GetStoreHeroSectionResponse
    {
        QString theme;
        QString title;
        struct
        {
            QString link;
            QString text;
            QString anchor;
        } button;
        QString imageHash;
        QDateTime endDate;
        QString videoId;
        QString subtitle;
        QString sectionId;
        QString description;
        bool showCountdown;
        struct
        {
            QString link;
            QString text;
            QString anchor;
        } buttonSecondary;
        bool showTopModLabel;
    };

    struct GetStoreHighlightsSectionResponse
    {
        QVector<StoreBannerItem> items;
    };

    struct GetStoreProductsSectionResponse
    {
        QString sectionId;
        QString contentSourceType;
        QString title;
        QString description;
        QVector<CatalogProduct> items;
        QString seeMoreLink;
    };

    struct GetStoreNewsSectionResponse
    {
        QVector<NewsItem> items;
    };

    struct GetStoreNowOnSaleResponse
    {
        QVector<CatalogProduct> products;
        QVector<StoreNowOnSaleTab> tabs;
    };

    struct GetStoreNowOnSaleSectionResponse
    {
        QString id;
        QVector<StoreProduct> personalizedProducts;
        StoreNowOnSaleTabCard bigThingy;
    };

    struct GetStorePromoBannerSectionResponse
    {
        QString sectionId;
        bool hideOnLoad;
        QString buttonText;
        QString link;
        QString image;
        QString logo;
        bool compact;
    };

    struct GetStoreRankingSectionResponse
    {
        QString sectionId;
        QString contentSourceType;
        QString title;
        QString description;
        QVector<CatalogProduct> items;
    };

    struct GetStoreSectionsResponse
    {
        struct
        {
            QString type;
            QDateTime endDate;
            QString pageUrl;
            QString promoId;
            QDateTime startDate;
        } config;
        QVector<StoreSection> sections;
    };

    struct GetStoreVerticalBannerSectionResponse
    {
        QVector<StoreVerticalBannerItem> items;
    };
}

#endif // STORE_H
