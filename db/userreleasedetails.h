#ifndef USERRELEASEDETAILS_H
#define USERRELEASEDETAILS_H

#include <QDate>
#include <QDateTime>
#include <QString>
#include <QVariant>
#include <QVector>

namespace db
{
    struct UserReleaseShortDetails
    {
        QString id;
        QString gameId;
        QString platformId;
        QString externalId;
        QString title;
        QDate releaseDate;
        QString verticalCover;
        QString icon;
        std::optional<double> aggregatedRating;
        bool owned;
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

    struct UserReleaseGroup
    {
        QVariant discriminator;
        QVector<UserReleaseShortDetails> items;
    };
}

#endif // USERRELEASEDETAILS_H
