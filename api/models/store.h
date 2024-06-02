#ifndef STORE_H
#define STORE_H

#include <QString>
#include <QVector>

namespace api
{
    struct StoreDiscoveredProductPrice
    {
        double baseAmount;
        double finalAmount;
        unsigned char discountPercentage;
        bool free;
    };

    struct StoreDiscoveredProduct
    {
        unsigned long long id;
        QString title;
        QString image;
        StoreDiscoveredProductPrice price;
        QVector<QString> supportedOperatingSystems;

        bool comingSoon;
        bool inDevelopment;
        bool availableForSale;
        bool buyable;
        bool movie;
        bool visibleInCatalog;
        bool preorder;
    };

    struct GetStoreDiscoverGamesSectionResponse
    {
        QVector<StoreDiscoveredProduct> personalizedProducts;
    };
}

#endif // STORE_H
