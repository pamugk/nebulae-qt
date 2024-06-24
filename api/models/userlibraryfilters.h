#ifndef USERLIBRARYFILTERS_H
#define USERLIBRARYFILTERS_H

#include <QVector>

#include "gamemetadata.h"
#include "metatag.h"

namespace api
{
    struct UserLibraryFilters
    {
        QVector<api::LocalizedMetaTag> genres;
        QVector<api::MetaTag> operatingSystems;
        bool noRating;
        QVector<QString> platforms;
        QVector<int> ratings;
        QVector<QString> tags;
    };
}

#endif // USERLIBRARYFILTERS_H
