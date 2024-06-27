#ifndef USERRELEASE_H
#define USERRELEASE_H

#include <QDateTime>
#include <QString>
#include <QStringList>
#include <QVector>

namespace api
{
    struct UserRelease
    {
        QString platformId;
        QString externalId;
        QStringList tags;
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
