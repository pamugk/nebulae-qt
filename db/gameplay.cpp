#include "database.h"

#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>

const auto INSERT_PLATFORM_RELEASE_ACHIEVEMENT = QLatin1String(R"(
INSERT INTO achievement(
    platform, platform_release_id, api_key, id,
    image_unlocked_url, image_locked_url, visible, rarity, rarity_slug)
    VALUES(?, ?, ?, ?, ?, ?, ?, ?, ?)
    ON CONFLICT(platform, platform_release_id, api_key) DO UPDATE
    SET id = excluded.id, image_unlocked_url = excluded.image_unlocked_url, image_locked_url = excluded.image_locked_url,
        visible = excluded.visible, rarity = excluded.rarity, rarity_slug = excluded.rarity_slug;
)");

const auto INSERT_PLATFORM_RELEASE_ACHIEVEMENT_LOCALIZED_DESCRIPTION = QLatin1String(R"(
INSERT INTO achievement_localized_description(platform, platform_release_id, api_key, locale_code, name, description, localized)
    VALUES(?, ?, ?, ?, ?, ?, ?)
    ON CONFLICT(platform, platform_release_id, api_key, locale_code) DO UPDATE
    SET name = excluded.name, description = excluded.description, localized = excluded.localized;
)");

const auto INSERT_PLATFORM_RELEASE_LAST_ACHIEVEMENTS_UPDATE = QLatin1String(R"(
INSERT INTO platform_release_last_achievements_update(platform, platform_release_id, last_updated_at)
    VALUES(?, ?, ?)
    ON CONFLICT(platform, platform_release_id) DO UPDATE
    SET last_updated_at = excluded.last_updated_at;
)");

const auto INSERT_USER_RELEASE_ACHIEVEMENT = QLatin1String(R"(
INSERT INTO user_release_achievement(user_id, platform, platform_release_id, api_key, unlocked_at)
    VALUES(?, ?, ?, ?, ?)
    ON CONFLICT(user_id, platform, platform_release_id, api_key) DO NOTHING;
)");

const auto INSERT_USER_RELEASE_GAME_TIME_STAT = QLatin1String(R"(
INSERT INTO user_release_game_time_stats(user_id, platform, platform_release_id, time_sum, last_session_at)
    VALUES(?, ?, ?, ?, ?)
    ON CONFLICT(user_id, platform, platform_release_id) DO UPDATE
    SET time_sum = excluded.time_sum, last_session_at = excluded.last_session_at;
)");

void db::savePlatformReleaseAchievements(const QString &platformId, const QString &platformReleaseId,
                                     const QVector<api::PlatformAchievement> &achievements)
{
    QSqlDatabase db = QSqlDatabase::database();
    if (!db.transaction())
    {
        qDebug() << "Failed to start DB transaction" << db.lastError();
        return;
    }

    QSqlQuery insertPlatformReleaseAchievementUpdateQuery;
    if (!insertPlatformReleaseAchievementUpdateQuery.prepare(INSERT_PLATFORM_RELEASE_LAST_ACHIEVEMENTS_UPDATE))
    {
        db.rollback();
        qDebug() << "Failed to prepare query to save platform release achievements update status" << insertPlatformReleaseAchievementUpdateQuery.lastError();
        return;
    }
    insertPlatformReleaseAchievementUpdateQuery.addBindValue(platformId);
    insertPlatformReleaseAchievementUpdateQuery.addBindValue(platformReleaseId);
    insertPlatformReleaseAchievementUpdateQuery.addBindValue(QDateTime::currentDateTimeUtc());

    if (!insertPlatformReleaseAchievementUpdateQuery.exec())
    {
        db.rollback();
        qDebug() << "Failed to update platform release achievements update status" << insertPlatformReleaseAchievementUpdateQuery.lastError();
        return;
    }

    QSqlQuery insertPlatformReleaseAchievementsQuery;
    if (!insertPlatformReleaseAchievementsQuery.prepare(INSERT_PLATFORM_RELEASE_ACHIEVEMENT))
    {
        db.rollback();
        qDebug() << "Failed to prepare query to save platform release achievements" << insertPlatformReleaseAchievementsQuery.lastError();
        return;
    }
    QSqlQuery insertPlatformReleaseAchievementsDescriptionsQuery;
    if (!insertPlatformReleaseAchievementsDescriptionsQuery.prepare(INSERT_PLATFORM_RELEASE_ACHIEVEMENT_LOCALIZED_DESCRIPTION))
    {
        db.rollback();
        qDebug() << "Failed to prepare query to save platform release achievements descriptions" << insertPlatformReleaseAchievementsDescriptionsQuery.lastError();
        return;
    }

    QVariantList platformIds(achievements.count(), platformId);
    QVariantList platformReleaseIds(achievements.count(), platformReleaseId);
    QVariantList apiKeys(achievements.count());
    QVariantList ids(achievements.count());
    QVariantList imageUnlockedUrls(achievements.count());
    QVariantList imageLockedUrls(achievements.count());
    QVariantList visible(achievements.count());
    QVariantList rarities(achievements.count());
    QVariantList raritySlugs(achievements.count());
    QVariantList localeCodes(achievements.count());
    QVariantList names(achievements.count());
    QVariantList descriptions(achievements.count());
    QVariantList localized(achievements.count());

    for (std::size_t i = 0; i < achievements.count(); i++)
    {
        const auto &achievement = achievements[i];
        apiKeys[i] = achievement.apiKey;
        ids[i] = achievement.id;
        imageUnlockedUrls[i] = achievement.imageUnlockedUrl;
        imageLockedUrls[i] = achievement.imageLockedUrl;
        visible[i] = achievement.visible;
        rarities[i] = achievement.rarity;
        raritySlugs[i] = achievement.rarityLevelSlug;
        localeCodes[i] = achievement.locale;
        names[i] = achievement.name;
        descriptions[i] = achievement.description;
        localized[i] = achievement.localized;
    }

    insertPlatformReleaseAchievementsQuery.addBindValue(platformIds);
    insertPlatformReleaseAchievementsQuery.addBindValue(platformReleaseIds);
    insertPlatformReleaseAchievementsQuery.addBindValue(apiKeys);
    insertPlatformReleaseAchievementsQuery.addBindValue(ids);
    insertPlatformReleaseAchievementsQuery.addBindValue(imageUnlockedUrls);
    insertPlatformReleaseAchievementsQuery.addBindValue(imageLockedUrls);
    insertPlatformReleaseAchievementsQuery.addBindValue(visible);
    insertPlatformReleaseAchievementsQuery.addBindValue(rarities);
    insertPlatformReleaseAchievementsQuery.addBindValue(raritySlugs);
    if (!insertPlatformReleaseAchievementsQuery.execBatch())
    {
        db.rollback();
        qDebug() << "Failed to save platform release achievements" << insertPlatformReleaseAchievementsQuery.lastError();
        return;
    }

    insertPlatformReleaseAchievementsDescriptionsQuery.addBindValue(platformIds);
    insertPlatformReleaseAchievementsDescriptionsQuery.addBindValue(platformReleaseIds);
    insertPlatformReleaseAchievementsDescriptionsQuery.addBindValue(apiKeys);
    insertPlatformReleaseAchievementsDescriptionsQuery.addBindValue(localeCodes);
    insertPlatformReleaseAchievementsDescriptionsQuery.addBindValue(names);
    insertPlatformReleaseAchievementsDescriptionsQuery.addBindValue(descriptions);
    insertPlatformReleaseAchievementsDescriptionsQuery.addBindValue(localized);
    if (!insertPlatformReleaseAchievementsDescriptionsQuery.execBatch())
    {
        db.rollback();
        qDebug() << "Failed to save platform release achievements descriptions" << insertPlatformReleaseAchievementsDescriptionsQuery.lastError();
        return;
    }

    db.commit();
}
void db::saveUserGameTimeStatistics(const QString &userId, const QVector<api::PlatformReleaseTimeStatistics> &statistics)
{
    QSqlDatabase db = QSqlDatabase::database();
    QSqlQuery insertUserGameStatsQuery;
    if (!insertUserGameStatsQuery.prepare(INSERT_USER_RELEASE_GAME_TIME_STAT))
    {
        qDebug() << "Failed to prepare query to save user's gameplay statistics" << insertUserGameStatsQuery.lastError();
        return;
    }

    QVariantList userIds(statistics.count(), userId);
    QVariantList platformIds(statistics.count());
    QVariantList platformReleaseIds(statistics.count());
    QVariantList timeSum(statistics.count());
    QVariantList lastPlayedAt(statistics.count());
    for (std::size_t i = 0; i < statistics.count(); i++)
    {
        const auto &releaseStatistic = statistics[i];
        const auto releaseKeySeparatorIdx = releaseStatistic.releasePerPlatformId.indexOf('_');
        if (releaseKeySeparatorIdx == -1)
        {
            qDebug() << "Platform release key with an unknown separator: " << releaseStatistic.releasePerPlatformId;
            platformIds[i] = "";
            platformReleaseIds[i] = "";
        }
        else
        {
            platformIds[i] = releaseStatistic.releasePerPlatformId.first(releaseKeySeparatorIdx);
            platformReleaseIds[i] = releaseStatistic.releasePerPlatformId.last(releaseStatistic.releasePerPlatformId.length() -  releaseKeySeparatorIdx - 1);
        }

        timeSum[i] = releaseStatistic.timeSum;
        lastPlayedAt[i] = releaseStatistic.lastSessionDate;
    }

    insertUserGameStatsQuery.addBindValue(userIds);
    insertUserGameStatsQuery.addBindValue(platformIds);
    insertUserGameStatsQuery.addBindValue(platformReleaseIds);
    insertUserGameStatsQuery.addBindValue(timeSum);
    insertUserGameStatsQuery.addBindValue(lastPlayedAt);
    if (!insertUserGameStatsQuery.execBatch())
    {
        qDebug() << "Failed to save user's gameplay statistics" << insertUserGameStatsQuery.lastError();
    }
}

void db::saveUserPlatformAchievements(const QString &userId, const QVector<api::PlatformUserAchievement> &achievements)
{
    QSqlDatabase db = QSqlDatabase::database();
    if (!db.transaction())
    {
        qDebug() << "Failed to start DB transaction" << db.lastError();
        return;
    }

    QSqlQuery insertPlatformReleaseAchievementsQuery;
    if (!insertPlatformReleaseAchievementsQuery.prepare(INSERT_PLATFORM_RELEASE_ACHIEVEMENT))
    {
        db.rollback();
        qDebug() << "Failed to prepare query to save platform release achievements" << insertPlatformReleaseAchievementsQuery.lastError();
        return;
    }
    QSqlQuery insertPlatformReleaseAchievementsDescriptionsQuery;
    if (!insertPlatformReleaseAchievementsDescriptionsQuery.prepare(INSERT_PLATFORM_RELEASE_ACHIEVEMENT_LOCALIZED_DESCRIPTION))
    {
        db.rollback();
        qDebug() << "Failed to prepare query to save platform release achievements descriptions" << insertPlatformReleaseAchievementsDescriptionsQuery.lastError();
        return;
    }
    QSqlQuery insertUserReleaseAchievementsQuery;
    if (!insertUserReleaseAchievementsQuery.prepare(INSERT_USER_RELEASE_ACHIEVEMENT))
    {
        db.rollback();
        qDebug() << "Failed to prepare query to save user release achievements" << insertUserReleaseAchievementsQuery.lastError();
        return;
    }

    QVariantList userIds(achievements.count(), userId);
    QVariantList platformIds(achievements.count());
    QVariantList platformReleaseIds(achievements.count());
    QVariantList apiKeys(achievements.count());
    QVariantList ids(achievements.count());
    QVariantList imageUnlockedUrls(achievements.count());
    QVariantList imageLockedUrls(achievements.count());
    QVariantList visible(achievements.count());
    QVariantList rarities(achievements.count());
    QVariantList raritySlugs(achievements.count());
    QVariantList localeCodes(achievements.count());
    QVariantList names(achievements.count());
    QVariantList descriptions(achievements.count());
    QVariantList localized(achievements.count());
    QVariantList unlockedAt(achievements.count());

    for (std::size_t i = 0; i < achievements.count(); i++)
    {
        const auto &achievement = achievements[i];
        const auto releaseKeySeparatorIdx = achievement.platformReleaseId.indexOf('_');
        if (releaseKeySeparatorIdx == -1)
        {
            qDebug() << "Platform release key with an unknown separator: " << achievement.platformReleaseId;
            platformIds[i] = "";
            platformReleaseIds[i] = "";
        }
        else
        {
            platformIds[i] = achievement.platformReleaseId.first(releaseKeySeparatorIdx);
            platformReleaseIds[i] = achievement.platformReleaseId.last(achievement.platformReleaseId.length() -  releaseKeySeparatorIdx - 1);
        }
        apiKeys[i] = achievement.apiKey;
        ids[i] = achievement.id;
        imageUnlockedUrls[i] = achievement.imageUnlockedUrl;
        imageLockedUrls[i] = achievement.imageLockedUrl;
        visible[i] = achievement.visible;
        rarities[i] = achievement.rarity;
        raritySlugs[i] = achievement.rarityLevelSlug;
        localeCodes[i] = achievement.locale;
        names[i] = achievement.name;
        descriptions[i] = achievement.description;
        localized[i] = achievement.localized;
        unlockedAt[i] = achievement.dateUnlocked;
    }

    insertPlatformReleaseAchievementsQuery.addBindValue(platformIds);
    insertPlatformReleaseAchievementsQuery.addBindValue(platformReleaseIds);
    insertPlatformReleaseAchievementsQuery.addBindValue(apiKeys);
    insertPlatformReleaseAchievementsQuery.addBindValue(ids);
    insertPlatformReleaseAchievementsQuery.addBindValue(imageUnlockedUrls);
    insertPlatformReleaseAchievementsQuery.addBindValue(imageLockedUrls);
    insertPlatformReleaseAchievementsQuery.addBindValue(visible);
    insertPlatformReleaseAchievementsQuery.addBindValue(rarities);
    insertPlatformReleaseAchievementsQuery.addBindValue(raritySlugs);
    if (!insertPlatformReleaseAchievementsQuery.execBatch())
    {
        db.rollback();
        qDebug() << "Failed to save platform release achievements" << insertPlatformReleaseAchievementsQuery.lastError();
        return;
    }

    insertPlatformReleaseAchievementsDescriptionsQuery.addBindValue(platformIds);
    insertPlatformReleaseAchievementsDescriptionsQuery.addBindValue(platformReleaseIds);
    insertPlatformReleaseAchievementsDescriptionsQuery.addBindValue(apiKeys);
    insertPlatformReleaseAchievementsDescriptionsQuery.addBindValue(localeCodes);
    insertPlatformReleaseAchievementsDescriptionsQuery.addBindValue(names);
    insertPlatformReleaseAchievementsDescriptionsQuery.addBindValue(descriptions);
    insertPlatformReleaseAchievementsDescriptionsQuery.addBindValue(localized);
    if (!insertPlatformReleaseAchievementsDescriptionsQuery.execBatch())
    {
        db.rollback();
        qDebug() << "Failed to save platform release achievements descriptions" << insertPlatformReleaseAchievementsDescriptionsQuery.lastError();
        return;
    }

    insertUserReleaseAchievementsQuery.addBindValue(userIds);
    insertUserReleaseAchievementsQuery.addBindValue(platformIds);
    insertUserReleaseAchievementsQuery.addBindValue(platformReleaseIds);
    insertUserReleaseAchievementsQuery.addBindValue(apiKeys);
    insertUserReleaseAchievementsQuery.addBindValue(unlockedAt);
    if (!insertUserReleaseAchievementsQuery.execBatch())
    {
        db.rollback();
        qDebug() << "Failed to save user release achievements" << insertUserReleaseAchievementsQuery.lastError();
        return;
    }

    db.commit();
}
