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

void db::savePlatformReleaseAchievements(const QString &platformId, const QString &platformReleaseId,
                                     const QVector<api::PlatformAchievement> &achievements)
{
    QSqlDatabase db = QSqlDatabase::database();
    if (!db.transaction())
    {
        qDebug() << "Failed to start DB transaction";
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
