#ifndef STATISTICS_H
#define STATISTICS_H

#include <QtGlobal>

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
        quint32 totalCount;
        quint32 limit;
        quint32 pageToken;
        bool hasNextPage;
        QString achievementsMode;
        QVector<Achievement> items;
    };

    struct GetPlaySessionsResponse
    {
        quint32 totalSum;
        QMap<quint64, quint32> gameTime;
    };
}

#endif // STATISTICS_H
