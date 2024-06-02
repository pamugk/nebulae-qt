#ifndef FILTERS_H
#define FILTERS_H

#include <QVector>

#include "./metatag.h"

namespace api
{
    struct PriceRange
    {
        unsigned int min;
        unsigned int max;
        QString currency;
        unsigned char decimalPlaces;
    };

    struct ReleaseDateRange
    {
        unsigned short min;
        unsigned short max;
    };

    struct StoreFilters
    {
        ReleaseDateRange releaseDateRange;
        PriceRange priceRange;
        QVector<MetaTag> genres;
        QVector<MetaTag> languages;
        QVector<MetaTag> systems;
        QVector<MetaTag> tags;
        bool discounted;
        QVector<MetaTag> features;
        QVector<MetaTag> releaseStatuses;
        QVector<QString> types;
        QVector<HierarchicalMetaTag> fullGenresList;
        QVector<MetaTag> fullTagsList;
    };
}

#endif // FILTERS_H
