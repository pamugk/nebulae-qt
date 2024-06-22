#ifndef PLATFORMACHIEVEMENT_H
#define PLATFORMACHIEVEMENT_H

#include <QString>
#include <QVector>

namespace api
{
    struct PlatformAchievement
    {
        QString id;
        QString platformReleaseId;
        QString apiKey;
        QString name;
        QString description;
        QString locale;
        bool localized;
        QString imageUnlockedUrl;
        QString imageLockedUrl;
        bool visible;
        double rarity;
        QString rarityLevelSlug;
    };

    struct GetPlatformReleaseAchievementsResponse
    {
        unsigned int limit;
        QString pageToken;
        QVector<PlatformAchievement> items;
    };
}

#endif // PLATFORMACHIEVEMENT_H
