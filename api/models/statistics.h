#ifndef STATISTICS_H
#define STATISTICS_H

#include <QDateTime>
#include <QMap>
#include <QVector>

namespace api
{
    struct UserAchievement
    {
        QString achievementId;
        QString achievementKey;
        QString clientId;
        bool visible;
        QString name;
        QString description;
        QString imageUrlUnlocked;
        QString imageUrlLocked;
        double rarity;
        QDateTime dateUnlocked;
        QString rarityLevelDescription;
        QString rarityLevelSlug;
    };

    struct GetUserAchievementsResponse
    {
        unsigned int totalCount;
        unsigned int limit;
        unsigned int pageToken;
        bool hasNextPage;
        QString achievementsMode;
        QVector<UserAchievement> items;
    };

    struct PlatformReleaseTimeStatistics
    {
        QString releasePerPlatformId;
        unsigned int timeSum;
        QDateTime lastSessionDate;
    };

    struct GetUserGameTimeStatisticsResponse
    {
        unsigned int limit;
        QString pageToken;
        QVector<PlatformReleaseTimeStatistics> items;
    };
}

#endif // STATISTICS_H
