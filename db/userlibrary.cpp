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
SELECT id, name, slug
FROM genre
WHERE id IN (
    SELECT DISTINCT genre_id
    FROM user_release
        JOIN platform_release ON user_release.platform = platform_release.platform AND user_release.platform_release_id = platform_release.platform_release_id
        JOIN "release" ON platform_release.release_id = "release".id
        JOIN game ON "release".game_id = game.id
        JOIN game_genre ON game.id = game_genre.game_id
    WHERE user_release.user_id = ? AND "release"."type" = 'game' AND game.visible
)
ORDER BY name;
)");

const auto SELECT_USER_RELEASES_OS = QLatin1String(R"(
SELECT slug, name
FROM os
WHERE slug IN (
    SELECT DISTINCT os_slug
    FROM user_release
        JOIN platform_release ON user_release.platform = platform_release.platform AND user_release.platform_release_id = platform_release.platform_release_id
        JOIN "release" ON platform_release.release_id = "release".id
        JOIN game ON "release".game_id = game.id
        JOIN release_supported_os ON "release".id = release_supported_os.release_id
    WHERE user_release.user_id = ? AND "release"."type" = 'game' AND game.visible
)
ORDER BY LOWER(name) DESC;
)");

const auto SELECT_USER_RELEASES_PLATFORMS = QLatin1String(R"(
SELECT DISTINCT user_release.platform
FROM user_release
    JOIN platform_release ON user_release.platform = platform_release.platform AND user_release.platform_release_id = platform_release.platform_release_id
    JOIN "release" ON platform_release.release_id = "release".id
    JOIN game ON "release".game_id = game.id
    JOIN game_genre ON game.id = game_genre.game_id
WHERE user_release.user_id = ? AND "release"."type" = 'game' AND game.visible
ORDER BY user_release.platform;
)");

const auto SELECT_USER_RELEASES_RATINGS = QLatin1String(R"(
SELECT DISTINCT user_release.rating
FROM user_release
    JOIN platform_release ON user_release.platform = platform_release.platform AND user_release.platform_release_id = platform_release.platform_release_id
    JOIN "release" ON platform_release.release_id = "release".id
    JOIN game ON "release".game_id = game.id
    JOIN game_genre ON game.id = game_genre.game_id
WHERE user_release.user_id = ? AND "release"."type" = 'game' AND game.visible
ORDER BY rating DESC;
)");

const auto SELECT_USER_RELEASES_TAGS = QLatin1String(R"(
SELECT DISTINCT tag
FROM user_release_tag
    JOIN platform_release ON user_release_tag.platform = platform_release.platform AND user_release_tag.platform_release_id = platform_release.platform_release_id
    JOIN "release" ON platform_release.release_id = "release".id
    JOIN game ON "release".game_id = game.id
    JOIN game_genre ON game.id = game_genre.game_id
WHERE user_release_tag.user_id = ? AND "release"."type" = 'game' AND game.visible
ORDER BY tag;
)");

const auto SELECT_USER_RELEASES_TEMPLATE = QLatin1String(R"(
SELECT platform_release.platform, platform_release.platform_release_id, platform_release.release_id,
    "release".title, "release".game_id,
    game.vertical_cover, game.square_icon,
    user_release.rating, user_release_game_time_stats.time_sum, user_release_game_time_stats.last_session_at,
    (
        SELECT count(*)
        FROM achievement JOIN platform_release_last_achievements_update
            ON achievement.platform = platform_release_last_achievements_update.platform
            AND achievement.platform_release_id = platform_release_last_achievements_update.platform_release_id
        WHERE user_release.platform = achievement.platform
            AND user_release.platform_release_id = achievement.platform_release_id
    ) AS total_achievement_count,
    (
        SELECT count(*)
        FROM user_release_achievement
        WHERE user_release.user_id = user_release_achievement.user_id
            AND user_release.platform = user_release_achievement.platform
            AND user_release.platform_release_id = user_release_achievement.platform_release_id
    ) AS unlocked_achievement_count,
    NULLIF((
        SELECT group_concat(developer.name, ', ')
        FROM game_developer JOIN developer ON game_developer.developer_id = developer.id
        WHERE game_developer.game_id = game.id
    ), '') AS developers,
    NULLIF((
        SELECT group_concat(publisher.name, ', ')
        FROM game_publisher JOIN publisher ON game_publisher.publisher_id = publisher.id
        WHERE game_publisher.game_id = game.id
    ), '') AS publishers,
    NULLIF((
        SELECT group_concat(genre.name, ', ')
        FROM game_genre JOIN genre ON game_genre.genre_id = genre.id
        WHERE game_genre.game_id = game.id
    ), '') AS genres,
    NULLIF((
        SELECT group_concat(user_release_tag.tag, ', ')
        FROM user_release_tag
        WHERE user_release.user_id = user_release_tag.user_id
            AND user_release.platform = user_release_tag.platform
            AND user_release.platform_release_id = user_release_tag.platform_release_id
    ), '') AS tags
FROM user_release
    LEFT JOIN user_release_game_time_stats ON
        user_release.user_id = user_release_game_time_stats.user_id
        AND user_release.platform = user_release_game_time_stats.platform
        AND user_release.platform_release_id = user_release_game_time_stats.platform_release_id
    JOIN platform_release ON user_release.platform = platform_release.platform
        AND user_release.platform_release_id = platform_release.platform_release_id
    JOIN "release" ON platform_release.release_id = "release".id
    JOIN game ON "release".game_id = game.id
WHERE user_release.user_id = ? AND user_release.hidden = ? AND "release"."type" = 'game' AND game.visible
)");

QVector<db::UserReleaseShortDetails> db::getUserReleases(const QString &userId, const api::SearchUserReleasesRequest &request)
{
    QSqlDatabase db = QSqlDatabase::database();
    if (!db.transaction())
    {
        qDebug() << "Failed to start DB transaction" << db.lastError();
        return QVector<UserReleaseShortDetails>();
    }

    QVariantList parameters;
    parameters << userId << request.hidden;
    QString dbQueryText(SELECT_USER_RELEASES_TEMPLATE);
    if (!request.genres.isEmpty())
    {
        dbQueryText += R"( AND EXISTS (
    SELECT * FROM game_genre
    WHERE game_genre.game_id = game.id
        AND game_genre.genre_id IN ()";
        bool firstParam = true;
        for (const auto &genre : std::as_const(request.genres))
        {
            if (firstParam)
            {
                dbQueryText += '?';
            }
            else
            {
                dbQueryText += ", ?";
            }
            firstParam = false;
            parameters << genre;
        }
        dbQueryText += "))";
    }
    if (!request.platforms.isEmpty())
    {
        dbQueryText += " AND user_release.platform IN (";
        bool firstParam = true;
        for (const auto &platform : std::as_const(request.platforms))
        {
            if (firstParam)
            {
                dbQueryText += '?';
            }
            else
            {
                dbQueryText += ", ?";
            }
            firstParam = false;
            parameters << platform;
        }
        dbQueryText += ')';
    }
    if (!request.supportedOs.isEmpty())
    {
        dbQueryText += R"( AND EXISTS (
    SELECT * FROM release_supported_os
    WHERE release_supported_os.release_id = "release".id
        AND release_supported_os.os_slug IN ()";
        bool firstParam = true;
        for (const auto &os : std::as_const(request.supportedOs))
        {
            if (firstParam)
            {
                dbQueryText += '?';
            }
            else
            {
                dbQueryText += ", ?";
            }
            firstParam = false;
            parameters << os;
        }
        dbQueryText += "))";
    }
    if (!request.ratings.isEmpty() || request.noRating)
    {
        if (request.ratings.isEmpty())
        {
            dbQueryText += " AND user_release.rating IS NULL";
        }
        else
        {
            dbQueryText += " AND (user_release.rating IN (";
            bool firstParam = true;
            for (const auto &rating : std::as_const(request.ratings))
            {
                if (firstParam)
                {
                    dbQueryText += '?';
                }
                else
                {
                    dbQueryText += ", ?";
                }
                firstParam = false;
                parameters << rating;
            }
            dbQueryText += ')';
            if (request.noRating)
            {
                dbQueryText += " OR user_release.rating IS NULL";
            }
            dbQueryText += ')';
        }
    }
    if (!request.tags.isEmpty())
    {
        dbQueryText += R"( AND EXISTS (
    SELECT * FROM user_release_tag
    WHERE user_release_tag.user_id = user_release.user_id
        AND user_release_tag.platform = user_release.platform
        AND user_release_tag.platform_release_id = user_release.platform_release_id
        AND user_release_tag.tag IN ()";
        bool firstParam = true;
        for (const auto &tag : std::as_const(request.tags))
        {
            if (firstParam)
            {
                dbQueryText += '?';
            }
            else
            {
                dbQueryText += ", ?";
            }
            firstParam = false;
            parameters << tag;
        }
        dbQueryText += "))";
    }
    if (request.owned)
    {
        dbQueryText += " AND user_release.owned";
    }
    if (!request.query.isEmpty())
    {
        dbQueryText += " AND LOWER(\"release\".title) GLOB ?";
        parameters << "*" + request.query.toLower() + "*";
    }
    switch (request.order)
    {
    case api::ACHIEVEMENTS:
        dbQueryText += " ORDER BY (unlocked_achievement_count / total_achievement_count) DESC, LOWER(\"release\".title_sort)";
        break;
    case api::CRITICS_RATING:
        dbQueryText += " ORDER BY game.aggregated_rating DESC, LOWER(\"release\".title_sort)";
        break;
    case api::GAME_TIME:
        dbQueryText += " ORDER BY user_release_game_time_stats.time_sum DESC, LOWER(\"release\".title_sort)";
        break;
    case api::GENRE:
        dbQueryText += " ORDER BY genres NULLS LAST, LOWER(\"release\".title_sort)";
        break;
    case api::LAST_PLAYED:
        dbQueryText += " ORDER BY user_release_game_time_stats.last_played_at DESC, LOWER(\"release\".title_sort)";
        break;
    case api::PLATFORM:
        dbQueryText += " ORDER BY user_release.platform, LOWER(\"release\".title_sort)";
        break;
    case api::RATING:
        dbQueryText += " ORDER BY user_release.rating DESC, LOWER(\"release\".title_sort)";
        break;
    case api::RELEASE_DATE:
        dbQueryText += " ORDER BY \"release\".first_release_date DESC, LOWER(\"release\".title_sort)";
        break;
    case api::SIZE_ON_DISK:
        dbQueryText += " ORDER BY LOWER(\"release\".title_sort)";
        break;
    case api::TAG:
        dbQueryText += " ORDER BY tags NULLS LAST, LOWER(\"release\".title_sort)";
        break;
    case api::TITLE:
        dbQueryText += " ORDER BY LOWER(\"release\".title_sort)";
        break;
    }
    dbQueryText += ';';
    QSqlQuery selectUserReleasesQuery;
    if (!selectUserReleasesQuery.prepare(dbQueryText))
    {
        db.rollback();
        qDebug() << "Failed to prepare query to select user releases" << selectUserReleasesQuery.lastError();
        return QVector<UserReleaseShortDetails>();
    }
    for (const QVariant &parameter : std::as_const(parameters))
    {
        selectUserReleasesQuery.addBindValue(parameter);
    }
    if (!selectUserReleasesQuery.exec())
    {
        db.rollback();
        qDebug() << "Failed to select user releases";
        return QVector<UserReleaseShortDetails>();
    }

    QVector<UserReleaseShortDetails> releases;
    while (selectUserReleasesQuery.next())
    {
        UserReleaseShortDetails release;
        release.platformId = selectUserReleasesQuery.value(0).toString();
        release.externalId = selectUserReleasesQuery.value(1).toString();
        release.id = selectUserReleasesQuery.value(2).toString();
        release.title = selectUserReleasesQuery.value(3).toString();
        release.gameId = selectUserReleasesQuery.value(4).toString();
        release.verticalCover = selectUserReleasesQuery.value(5).toString();
        release.icon = selectUserReleasesQuery.value(6).toString();
        if (!selectUserReleasesQuery.value(7).isNull())
        {
            release.rating = selectUserReleasesQuery.value(7).toInt();
        }
        release.totalPlaytime = selectUserReleasesQuery.value(8).toUInt();
        release.lastPlayedAt = selectUserReleasesQuery.value(9).toDateTime();
        release.totalAchievementCount = selectUserReleasesQuery.value(10).toUInt();
        release.unlockedAchievementCount = selectUserReleasesQuery.value(11).toUInt();
        release.developers = selectUserReleasesQuery.value(12).toString();
        release.publishers = selectUserReleasesQuery.value(13).toString();
        release.genres = selectUserReleasesQuery.value(14).toString();
        release.tags = selectUserReleasesQuery.value(15).toString();
        releases << release;
    }

    db.commit();

    return releases;
}

api::UserLibraryFilters db::getUserReleasesFilters(const QString &userId)
{
    QSqlDatabase db = QSqlDatabase::database();

    if (!db.transaction())
    {
        qDebug() << "Failed to start DB transaction" << db.lastError();
        return {};
    }

    api::UserLibraryFilters filters;

    QSqlQuery selectUserReleasesGenresQuery;
    if (selectUserReleasesGenresQuery.prepare(SELECT_USER_RELEASES_GENRES))
    {
        selectUserReleasesGenresQuery.bindValue(0, userId);
        if (selectUserReleasesGenresQuery.exec())
        {
            while (selectUserReleasesGenresQuery.next())
            {
                api::LocalizedMetaTag genre;
                genre.id = selectUserReleasesGenresQuery.value(0).toString();
                genre.name["*"] = selectUserReleasesGenresQuery.value(1).toString();
                genre.slug = selectUserReleasesGenresQuery.value(2).toString();
                filters.genres << genre;
            }
        }
        else
        {
            qDebug() << "Failed to select genres of user releases"
                     << selectUserReleasesGenresQuery.lastError();
        }
    }
    else
    {
        qDebug() << "Failed to prepare query to select genres of user releases"
                 << selectUserReleasesGenresQuery.lastError();
    }

    QSqlQuery selectUserReleasesPlatformsQuery;
    if (selectUserReleasesPlatformsQuery.prepare(SELECT_USER_RELEASES_PLATFORMS))
    {
        selectUserReleasesPlatformsQuery.bindValue(0, userId);
        if (selectUserReleasesPlatformsQuery.exec())
        {
            while (selectUserReleasesPlatformsQuery.next())
            {
                filters.platforms << selectUserReleasesPlatformsQuery.value(0).toString();
            }
        }
        else
        {
            qDebug() << "Failed to select user releases platforms"
                     << selectUserReleasesPlatformsQuery.lastError();
        }
    }
    else
    {
        qDebug() << "Failed to prepare query to select user releases platforms"
                 << selectUserReleasesPlatformsQuery.lastError();
    }

    QSqlQuery selectUserReleasesOsQuery;
    if (selectUserReleasesOsQuery.prepare(SELECT_USER_RELEASES_OS))
    {
        selectUserReleasesOsQuery.bindValue(0, userId);
        if (selectUserReleasesOsQuery.exec())
        {
            while (selectUserReleasesOsQuery.next())
            {
                api::MetaTag os;
                os.slug = selectUserReleasesOsQuery.value(0).toString();
                os.name = selectUserReleasesOsQuery.value(1).toString();
                filters.operatingSystems << os;
            }
        }
        else
        {
            qDebug() << "Failed to select supported OS of user releases"
                     << selectUserReleasesOsQuery.lastError();
        }
    }
    else
    {
        qDebug() << "Failed to prepare query to select supported OS of user releases"
                 << selectUserReleasesOsQuery.lastError();
    }

    QSqlQuery selectUserReleasesRatingsQuery;
    if (selectUserReleasesRatingsQuery.prepare(SELECT_USER_RELEASES_RATINGS))
    {
        selectUserReleasesRatingsQuery.bindValue(0, userId);
        if (selectUserReleasesRatingsQuery.exec())
        {
            while (selectUserReleasesRatingsQuery.next())
            {
                auto rating = selectUserReleasesRatingsQuery.value(0);
                if (rating.isNull())
                {
                    filters.noRating = true;
                }
                else
                {
                    filters.ratings << rating.toInt();
                }
            }
        }
        else
        {
            qDebug() << "Failed to select user releases ratings"
                     << selectUserReleasesRatingsQuery.lastError();
        }
    }
    else
    {
        qDebug() << "Failed to prepare query to select user releases ratings"
                 << selectUserReleasesRatingsQuery.lastError();
    }

    QSqlQuery selectUserReleasesTagsQuery;
    if (selectUserReleasesTagsQuery.prepare(SELECT_USER_RELEASES_TAGS))
    {
        selectUserReleasesTagsQuery.bindValue(0, userId);
        if (selectUserReleasesTagsQuery.exec())
        {
            while (selectUserReleasesTagsQuery.next())
            {
                filters.tags << selectUserReleasesTagsQuery.value(0).toString();
            }
        }
        else
        {
            qDebug() << "Failed to select user releases tags"
                     << selectUserReleasesTagsQuery.lastError();
        }
    }
    else
    {
        qDebug() << "Failed to prepare query to select user releases tags"
                 << selectUserReleasesTagsQuery.lastError();
    }

    db.commit();

    return filters;
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
            insertReleaseTagQuery.bindValue(0, userId);
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
