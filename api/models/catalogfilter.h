#ifndef CATALOGFILTERS_H
#define CATALOGFILTERS_H

#include <QString>
#include <QStringList>

namespace api
{
    struct CatalogFilter
    {
        bool discounted;
        bool free;
        bool hideOwned;
        bool onlyDlcForOwned;
        bool onlyWishlisted;
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
        // Hidden filters available only via redirections
        QStringList developers;
        QStringList publishers;
    };
}

#endif // CATALOGFILTERS_H
