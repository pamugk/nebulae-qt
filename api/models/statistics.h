#ifndef STATISTICS_H
#define STATISTICS_H

#include <QDateTime>
#include <QMap>
#include <QVector>

#include "platformachievement.h"

namespace api
{
    struct PlatformUserAchievement : PlatformAchievement
    {
        QDateTime dateUnlocked;
    };

    struct GetUserPlatformAchievementsResponse
    {
        unsigned int limit;
        QString pageToken;
        QString nextPageToken;
        QVector<PlatformUserAchievement> items;
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
