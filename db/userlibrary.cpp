#include "database.h"

#include <QDebug>
#include <QSet>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlRecord>

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

const auto SELECT_USER_RELEASES_GENRES = QLatin1String(R"(
SELECT genre.id, genre.name, genre.slug
FROM (
    SELECT DISTINCT genre_id
    FROM game_genre
    WHERE game_id IN (
        SELECT game_id
        FROM user_release
            JOIN platform_release ON user_release.platform = platform_release.platform AND user_release.platform_release_id = platform_release.platform_release_id
            JOIN "release" ON platform_release.release_id = "release".id
            JOIN game ON "release".game_id = game.id
        WHERE user_release.user_id = ? AND "release"."type" = 'game' AND game.visible
    )
) tmp JOIN genre ON tmp.genre_id = genre.id;
)");

const auto SELECT_USER_RELEASES_OS = QLatin1String(R"(
SELECT os.slug, os.name
FROM (
    SELECT DISTINCT os_slug
    FROM release_supported_os
    WHERE release_id IN (
        SELECT release_id
        FROM user_release
            JOIN platform_release ON user_release.platform = platform_release.platform AND user_release.platform_release_id = platform_release.platform_release_id
            JOIN "release" ON platform_release.release_id = "release".id
            JOIN game ON "release".game_id = game.id
        WHERE user_release.user_id = ? AND "release"."type" = 'game' AND game.visible
    )
) tmp JOIN os ON tmp.os_slug = os.slug;
)");

const auto SELECT_USER_RELEASES_TEMPLATE = QLatin1String(R"(
SELECT platform_release.platform, platform_release.platform_release_id, platform_release.release_id,
    "release".title, "release".game_id,
    game.vertical_cover, game.cover
FROM user_release
    JOIN platform_release ON user_release.platform = platform_release.platform
        AND user_release.platform_release_id = platform_release.platform_release_id
    JOIN "release" ON platform_release.release_id = "release".id
    JOIN game ON "release".game_id = game.id
WHERE user_release.user_id = ? AND user_release.hidden = ? AND "release"."type" = 'game' AND game.visible
)");

QVector<api::Release> db::getUserReleases(const QString &userId, const api::SearchUserReleasesRequest &request)
{
    QSqlDatabase db = QSqlDatabase::database();
    if (!db.transaction())
    {
        qDebug() << "Failed to start DB transaction" << db.lastError();
        return QVector<api::Release>();
    }

    QVariantList parameters;
    parameters << userId << request.hidden;
    QString dbQueryText(SELECT_USER_RELEASES_TEMPLATE);
    if (!request.query.isEmpty())
    {
        dbQueryText += " AND LOWER(\"release\".title) GLOB ?";
        parameters << "*" + request.query.toLower() + "*";
    }
    switch (request.order)
    {
    case api::ACHIEVEMENTS:
        // TODO: load gameplay statistics to implement this sort
    case api::CRITICS_RATING:
        dbQueryText += " ORDER BY game.aggregated_rating DESC, lower(\"release\".title_sort)";
        break;
    case api::GAME_TIME:
        // TODO: load gameplay statistics to implement this sort
    case api::GENRE:
    case api::LAST_PLAYED:
        // TODO: load gameplay statistics to implement this sortd
        dbQueryText += " ORDER BY lower(\"release\".title_sort)";
        break;
    case api::PLATFORM:
        dbQueryText += " ORDER BY user_release.platform, lower(\"release\".title_sort)";
        break;
    case api::RATING:
        dbQueryText += " ORDER BY user_release.rating DESC, lower(\"release\".title_sort)";
        break;
    case api::RELEASE_DATE:
        dbQueryText += " ORDER BY \"release\".first_release_date DESC, lower(\"release\".title_sort)";
        break;
    case api::SIZE_ON_DISK:
        // TODO: implement this sort order after installation
    case api::TAG:
    case api::TITLE:
        dbQueryText += " ORDER BY lower(\"release\".title_sort)";
        break;
    }
    dbQueryText += ';';
    QSqlQuery selectUserReleasesQuery;
    if (!selectUserReleasesQuery.prepare(dbQueryText))
    {
        db.rollback();
        qDebug() << "Failed to prepare query to select user releases" << selectUserReleasesQuery.lastError();
        return QVector<api::Release>();
    }
    for (const QVariant &parameter : std::as_const(parameters))
    {
        selectUserReleasesQuery.addBindValue(parameter);
    }
    if (!selectUserReleasesQuery.exec())
    {
        db.rollback();
        qDebug() << "Failed to select user releases";
        return QVector<api::Release>();
    }

    QVector<api::Release> releases;
    if (selectUserReleasesQuery.first())
    {
        while (selectUserReleasesQuery.next())
        {
            api::Release release;
            release.platformId = selectUserReleasesQuery.value(0).toString();
            release.externalId = selectUserReleasesQuery.value(1).toString();
            release.id = selectUserReleasesQuery.value(2).toString();
            release.title["*"] = selectUserReleasesQuery.value(3).toString();
            release.gameId = selectUserReleasesQuery.value(4).toString();
            release.game.verticalCover = selectUserReleasesQuery.value(5).toString();
            release.game.cover = selectUserReleasesQuery.value(6).toString();
            releases << release;
        }
    }

    db.commit();

    return releases;
}

QVector<api::LocalizedMetaTag> db::getUserReleasesGenres(const QString &userId)
{
    QSqlDatabase db = QSqlDatabase::database();
    QSqlQuery selectUserReleasesGenresQuery;
    if (!selectUserReleasesGenresQuery.prepare(SELECT_USER_RELEASES_GENRES))
    {
        qDebug() << "Failed to prepare query to select genres of user releases";
        return {};
    }
    selectUserReleasesGenresQuery.bindValue(0, userId);
    if (!selectUserReleasesGenresQuery.exec())
    {
        qDebug() << "Failed to select genres of user releases";
        return {};
    }
    QVector<api::LocalizedMetaTag> genres;
    if (selectUserReleasesGenresQuery.first())
    {
        while (selectUserReleasesGenresQuery.next())
        {
            api::LocalizedMetaTag genre;
            genre.id = selectUserReleasesGenresQuery.value(0).toString();
            genre.name["*"] = selectUserReleasesGenresQuery.value(1).toString();
            genre.slug = selectUserReleasesGenresQuery.value(2).toString();
            genres << genre;
        }
    }
    return genres;
}

QVector<api::MetaTag> db::getUserReeleasesOs(const QString &userId)
{
    QSqlDatabase db = QSqlDatabase::database();
    QSqlQuery selectUserReleasesOsQuery;
    if (!selectUserReleasesOsQuery.prepare(SELECT_USER_RELEASES_OS))
    {
        qDebug() << "Failed to prepare query to select supported OS of user releases";
        return {};
    }
    selectUserReleasesOsQuery.bindValue(0, userId);
    if (!selectUserReleasesOsQuery.exec())
    {
        qDebug() << "Failed to select supported OS of user releases";
        return {};
    }

    QVector<api::MetaTag> supportedOs;
    if (selectUserReleasesOsQuery.first())
    {
        while (selectUserReleasesOsQuery.next())
        {
            api::MetaTag os;
            os.slug = selectUserReleasesOsQuery.value(0).toString();
            os.name = selectUserReleasesOsQuery.value(1).toString();
            supportedOs << os;
        }
    }
    return supportedOs;
}

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
