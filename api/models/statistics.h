#ifndef STATISTICS_H
#define STATISTICS_H

#include <QDateTime>
#include <QMap>
#include <QVector>

namespace api
{
    struct Achievement
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

    struct GetAchievementsResponse
    {
        unsigned int totalCount;
        unsigned int limit;
        unsigned int pageToken;
        bool hasNextPage;
        QString achievementsMode;
        QVector<Achievement> items;
    };

    struct GetPlaySessionsResponse
    {
        unsigned int totalSum;
        QMap<quint64, quint32> gameTime;
    };
}

#endif // STATISTICS_H
