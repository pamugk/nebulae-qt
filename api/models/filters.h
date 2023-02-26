#ifndef FILTERS_H
#define FILTERS_H

#include <QVector>

#include "./metatag.h"

namespace api
{
    struct PriceRange
    {
        quint32 min;
        quint32 max;
        QString currency;
        quint8 decimalPlaces;
    };

    struct ReleaseDateRange
    {
        quint8 min;
        quint8 max;
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
