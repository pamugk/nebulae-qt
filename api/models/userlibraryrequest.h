#ifndef USERLIBRARYREQUEST_H
#define USERLIBRARYREQUEST_H

#include <QSet>
#include <QString>
#include <QVariant>

namespace api
{
    enum UserReleaseOrder
    {
        ACHIEVEMENTS,
        CRITICS_RATING,
        GAME_TIME,
        GENRE,
        LAST_PLAYED,
        PLATFORM,
        RATING,
        RELEASE_DATE,
        SIZE_ON_DISK,
        TAG,
        TITLE
    };

    enum UserReleaseGrouping
    {
        NO_GROUP,
        CRITICS_RATING_GROUP,
        GENRE_GROUP,
        INSTALLED_GROUP,
        PLATFORM_GROUP,
        RATING_GROUP,
        RELEASE_DATE_GROUP,
        SUBSCRIPTION_GROUP,
        TAG_GROUP
    };

    struct SearchUserReleasesRequest
    {
        // Filters
        QSet<QString> genres;
        bool hidden;
        QString query;
        bool noRating;
        bool owned;
        QSet<QString> platforms;
        QSet<int> ratings;
        QSet<QString> supportedOs;
        QSet<QString> tags;

        // Order & group clause
        UserReleaseOrder order;
        UserReleaseGrouping grouping;
    };
}

#endif // USERLIBRARYREQUEST_H
