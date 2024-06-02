#ifndef CATALOG_H
#define CATALOG_H

#include <QString>
#include <QVector>

#include "./filters.h"
#include "./metatag.h"

namespace api
{
    struct Money
    {
        QString amount;
        QString currency;
    };

    struct CatalogPrice
    {
        QString final;
        QString base;
        QString discount;
        Money finalMoney;
        Money baseMoney;
    };

    struct PreferredLanguage
    {
        QString code;
        bool inAudio;
        bool inText;
    };

    struct CatalogProduct
    {
        QString id;
        QString slug;
        QVector<MetaTag> features;
        QVector<QString> screenshots;
        PreferredLanguage userPreferredLanguage;
        QString releaseDate;
        QString productType;
        QString title;
        QString coverHorizontal;
        QString coverVertical;
        QVector<QString> developers;
        QVector<QString> publishers;
        QVector<QString> operatingSystems;
        CatalogPrice price;
        QString productState;
        QVector<MetaTag> genres;
        QVector<MetaTag> tags;
        unsigned char reviewsRating;
    };

    struct SearchCatalogResponse
    {   unsigned short pages;
        unsigned int productCount;
        QVector<CatalogProduct> products;
        StoreFilters filters;
    };
}

#endif // CATALOG_H
