#ifndef OWNEDPRODUCT_H
#define OWNEDPRODUCT_H

#include <QtGlobal>

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
        quint8 rating;

        quint64 id;
        QVector<QString> tags;
        QString title;
        QString image;
        QString url;
        QString category;
        QString slug;
        quint32 updates;
        quint32 dlcCount;
        ZonedDateTime releaseDate;
    };

    struct GetOwnedProductsResponse
    {
        quint16 moviesCount;
        quint16 productsPerPage;
        quint16 page;
        quint16 totalPages;
        quint32 totalProducts;
        QVector<Tag> tags;
        QVector<OwnedProduct> products;
        quint32 updatedProductsCount;
        quint32 hiddenUpdatedProductsCount;
        bool hasHiddenProducts;
    };
}

#endif // OWNEDPRODUCT_H
