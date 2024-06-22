#include "database.h"

#include <QDebug>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>

const auto INSERT_USER_RELEASE = QLatin1String(R"(
INSERT INTO user_release(user_id, platform, platform_release_id, created_at, owned_since, owned, hidden, rating)
    VALUES(?, ?, ?, ?, ?, ?, ?, ?)
    ON CONFLICT(user_id, platform, platform_release_id) DO UPDATE
        SET owned_since = excluded.owned_since, hidden = excluded.hidden, rating = excluded.rating;
)");

const auto INSERT_USER_RELEASE_TAG = QLatin1String(R"(
INSERT INTO user_release_tag(user_id, platform, platform_release_id, tag)
    VALUES(?, ?, ?, ?)
    ON CONFLICT DO NOTHING;
)");

const auto SELECT_ANY_USER_RELEASE_TO_MAP = QLatin1String(R"(
SELECT platform, platform_release_id
FROM user_release
WHERE user_id = ? AND NOT EXISTS (
    SELECT *
    FROM platform_release
    WHERE user_release.platform = platform_release.platform
        AND user_release.platform_release_id = platform_release.platform_release_id
)
LIMIT 1;
)");

const auto SELECT_ANY_USER_RELEASE_TO_UPDATE_ACHIEVEMENTS = QLatin1String(R"(
SELECT platform, platform_release_id
FROM user_release
WHERE user_id = ? AND NOT EXISTS (
    SELECT *
    FROM platform_release_last_achievements_update
    WHERE user_release.platform = platform_release_last_achievements_update.platform
        AND user_release.platform_release_id = platform_release_last_achievements_update.platform_release_id
)
LIMIT 1;
)");

std::tuple<QString, QString> db::getUserReleaseToMap(const QString &userId)
{
    QSqlDatabase db = QSqlDatabase::database();
    QSqlQuery selectUserReleaseToMapQuery;
    if (!selectUserReleaseToMapQuery.prepare(SELECT_ANY_USER_RELEASE_TO_MAP))
    {
        qDebug() << "Failed to prepare query to select any not mapped user release";
        return {};
    }
    selectUserReleaseToMapQuery.bindValue(0, userId);
    selectUserReleaseToMapQuery.exec();
    if (selectUserReleaseToMapQuery.next())
    {
        return std::make_tuple(selectUserReleaseToMapQuery.value(0).toString(), selectUserReleaseToMapQuery.value(1).toString());
    }
    else
    {
        return {};
    }
}

std::tuple<QString, QString> db::getUserReleaseToUpdateAchievements(const QString &userId)
{
    QSqlDatabase db = QSqlDatabase::database();
    QSqlQuery selectUserReleaseToMapQuery;
    if (!selectUserReleaseToMapQuery.prepare(SELECT_ANY_USER_RELEASE_TO_UPDATE_ACHIEVEMENTS))
    {
        qDebug() << "Failed to prepare query to select any not mapped user release";
        return {};
    }
    selectUserReleaseToMapQuery.bindValue(0, userId);
    if (!selectUserReleaseToMapQuery.exec())
    {
        qDebug() << "Failed to select any not mapped user release" << selectUserReleaseToMapQuery.lastError();
        return {};
    }
    if (selectUserReleaseToMapQuery.next())
    {
        return std::make_tuple(selectUserReleaseToMapQuery.value(0).toString(), selectUserReleaseToMapQuery.value(1).toString());
    }
    else
    {
        return {};
    }
}

void db::saveUserReleases(const QString &userId, const QVector<api::UserRelease> &releases)
{
    QSqlDatabase db = QSqlDatabase::database();
    QSqlQuery insertQuery;
    if (!insertQuery.prepare(INSERT_USER_RELEASE))
    {
        qDebug() << "Failed to prepare user releases persistence query";
        return;
    }
    QSqlQuery insertReleaseTagQuery;
    if (!insertReleaseTagQuery.prepare(INSERT_USER_RELEASE_TAG))
    {
        qDebug() << "Failed to prepare user releases tags persistence query";
        return;
    }
    for (const api::UserRelease &release : releases)
    {
        insertQuery.bindValue(0, userId);
        insertQuery.bindValue(1, release.platformId);
        insertQuery.bindValue(2, release.externalId);
        insertQuery.bindValue(3, release.dateCreated);
        insertQuery.bindValue(4, release.ownedSince);
        insertQuery.bindValue(5, release.owned);
        insertQuery.bindValue(6, release.hidden.has_value() ? release.hidden.value() : QVariant(QMetaType::fromType<bool>()));
        insertQuery.bindValue(7, release.rating.has_value() ? release.rating.value() : QVariant(QMetaType::fromType<int>()));
        if (!insertQuery.exec())
        {
            qDebug() << "Failed to persist release " << release.platformId << release.externalId << insertQuery.lastError();
        }

        for (const QString &tag : release.tags)
        {
            insertReleaseTagQuery.bindValue(0, 0);
            insertReleaseTagQuery.bindValue(1, release.platformId);
            insertReleaseTagQuery.bindValue(2, release.externalId);
            insertReleaseTagQuery.bindValue(3, tag);
            if (!insertReleaseTagQuery.exec())
            {
                qDebug() << "Failed to persist tag " << release.platformId << release.externalId << insertReleaseTagQuery.lastError();
            }
        }
    }
}
