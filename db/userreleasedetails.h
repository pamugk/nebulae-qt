#ifndef USERRELEASEDETAILS_H
#define USERRELEASEDETAILS_H

#include <QDateTime>
#include <QString>

namespace db
{
    struct UserReleaseShortDetails
    {
        QString id;
        QString gameId;
        QString platformId;
        QString externalId;
        QString title;
        QString verticalCover;
        QString icon;
        std::optional<int> rating;
        unsigned int totalPlaytime;
        QDateTime lastPlayedAt;
        unsigned int totalAchievementCount;
        unsigned int unlockedAchievementCount;
        QString developers;
        QString publishers;
        QString genres;
        QString tags;
    };
}

#endif // USERRELEASEDETAILS_H
