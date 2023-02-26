#ifndef CATALOGFILTERS_H
#define CATALOGFILTERS_H

#include <QString>
#include <QStringList>

namespace api
{
    struct CatalogFilter
    {
        bool discounted;
        bool hideOwned;
        QString query;
        QStringList genres;
        QStringList excludeGenres;
        QStringList languages;
        QStringList systems;
        QStringList tags;
        QStringList excludeTags;
        QStringList features;
        QStringList excludeFeatures;
        QStringList releaseStatuses;
        QStringList excludeReleaseStatuses;
        QStringList productTypes;
    };
}

#endif // CATALOGFILTERS_H
