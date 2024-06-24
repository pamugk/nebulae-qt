#ifndef USERRELEASE_H
#define USERRELEASE_H

#include <QDateTime>
#include <QString>
#include <QVector>

namespace api
{
    struct UserRelease
    {
        QString platformId;
        QString externalId;
        QVector<QString> tags;
        std::optional<int> rating;
        std::optional<bool> hidden;
        QDateTime dateCreated;
        QDateTime ownedSince;
        QString certificate;
        bool owned;
    };

    struct GetUserReleasesResponse
    {
        unsigned int totalCount;
        unsigned int limit;
        QVector<UserRelease> items;
    };
}

#endif // USERRELEASE_H
