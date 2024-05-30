#ifndef OWNEDPRODUCT_H
#define OWNEDPRODUCT_H

#include <QString>

#include "./productshared.h"
#include "./tag.h"

namespace api
{
    struct OwnedProduct
    {
        bool galaxyCompatible;
        bool comingSoon;
        bool movie;
        bool game;
        bool baseProductMissing;
        bool hiddingDisabled;
        bool inDevelopment;
        bool hidden;
        bool isNew;
        Availability availability;
        PlatformAvailability worksOn;
        unsigned char rating;

        unsigned long long id;
        QVector<QString> tags;
        QString title;
        QString image;
        QString url;
        QString category;
        QString slug;
        unsigned int updates;
        unsigned int dlcCount;
        ZonedDateTime releaseDate;
    };

    struct GetOwnedProductsResponse
    {
        unsigned short moviesCount;
        unsigned short productsPerPage;
        unsigned short page;
        unsigned short totalPages;
        unsigned int totalProducts;
        QVector<Tag> tags;
        QVector<OwnedProduct> products;
        unsigned int updatedProductsCount;
        unsigned int hiddenUpdatedProductsCount;
        bool hasHiddenProducts;
    };
}

#endif // OWNEDPRODUCT_H
