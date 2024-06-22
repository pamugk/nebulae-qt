#ifndef USERLIBRARYREQUEST_H
#define USERLIBRARYREQUEST_H

#include <QString>
#include <QVector>

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

    struct SearchUserReleasesRequest
    {
        // Filters
        QVector<QString> genres;
        bool hidden;
        QString query;
        QVector<QString> supportedOs;

        // Order & group clause
        UserReleaseOrder order;
    };
}

#endif // USERLIBRARYREQUEST_H
