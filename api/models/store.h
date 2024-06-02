#ifndef STORE_H
#define STORE_H

#include <QDateTime>
#include <QString>
#include <QVector>

namespace api
{
    struct StoreProductPrice
    {
        double baseAmount;
        double finalAmount;
        unsigned char discountPercentage;
        bool free;
    };

    struct StoreProduct
    {
        unsigned long long id;
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

    struct StoreCustomSectionItem
    {
        QDateTime dealActiveFrom;
        QDateTime dealActiveTo;
        StoreProduct product;
    };

    struct GetStoreCustomSectionResponse
    {
        QString id;
        QString title;
        QVector<StoreCustomSectionItem> items;

        QDateTime currentServerTime;
        QDateTime visibleFrom;
        QDateTime visibleTo;
    };

    struct GetStoreDiscoverGamesSectionResponse
    {
        QVector<StoreProduct> personalizedProducts;
    };
}

#endif // STORE_H
