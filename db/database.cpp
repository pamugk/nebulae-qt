 #include "database.h"

#include <QDebug>
#include <QDir>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QStandardPaths>

// LIBRARY DATABASE DDL

// Gameplay info
const auto ACHIEVEMENT_DDL = QLatin1String(R"(
CREATE TABLE IF NOT EXISTS achievement(
    platform TEXT NOT NULL,
    platform_release_id TEXT NOT NULL,
    api_key TEXT NOT NULL,
    id TEXT NULL,
    image_unlocked_url TEXT NULL,
    image_locked_url TEXT NULL,
    visible BOOLEAN NOT NULL,
    rarity REAL NULL,
    rarity_slug TEXT NULL,
    PRIMARY KEY (platform, platform_release_id, api_key));
)");

const auto ACHIEVEMENT_LOCALIZED_DESCRIPTION_DDL = QLatin1String(R"(
CREATE TABLE IF NOT EXISTS achievement_localized_description(
    platform TEXT NOT NULL,
    platform_release_id TEXT NOT NULL,
    api_key TEXT NOT NULL,
    locale_code TEXT NOT NULL,
    name TEXT NOT NULL,
    description TEXT NULL,
    localized BOOLEAN NOT NULL,
    PRIMARY KEY (platform, platform_release_id, api_key, locale_code),
    FOREIGN KEY (platform, platform_release_id, api_key) REFERENCES achievement (platform, platform_release_id, api_key) ON DELETE CASCADE);
)");

// GamesDB basic entities & value objects
const auto DEVELOPER_DDL = QLatin1String(R"(
CREATE TABLE IF NOT EXISTS developer(
    id TEXT PRIMARY KEY NOT NULL,
    name TEXT NOT NULL,
    slug TEXT NOT NULL);
)");

const auto GAME_DDL = QLatin1String(R"(
CREATE TABLE IF NOT EXISTS game(
    id TEXT PRIMARY KEY NOT NULL,
    parent_id TEXT NULL,
    first_release_date NOT NULL,
    title TEXT NOT NULL,
    title_sort TEXT NOT NULL,
    type TEXT NOT NULL,
    summary TEXT NULL,
    visible BOOLEAN NOT NULL,
    aggregated_rating REAL NULL,
    horizontal_artwork TEXT NULL,
    background TEXT NULL,
    vertical_cover TEXT NULL,
    cover TEXT NULL,
    logo TEXT NULL,
    square_icon TEXT NULL);
)");

const auto GAME_ARTWORK_DDL = QLatin1String(R"(
CREATE TABLE IF NOT EXISTS game_artwork(
    game_id TEXT NOT NULL,
    url TEXT NOT NULL,
    FOREIGN KEY (game_id) REFERENCES game (id) ON DELETE CASCADE,
    UNIQUE (game_id, url));
)");

const auto GAME_MODE_DDL = QLatin1String(R"(
CREATE TABLE IF NOT EXISTS game_mode(
    id TEXT PRIMARY KEY NOT NULL,
    name TEXT NOT NULL,
    slug TEXT NOT NULL);
)");

const auto GAME_SCREENSHOT_DDL = QLatin1String(R"(
CREATE TABLE IF NOT EXISTS game_screenshot(
    game_id TEXT NOT NULL,
    url TEXT NOT NULL,
    FOREIGN KEY (game_id) REFERENCES game (id) ON DELETE CASCADE,
    UNIQUE (game_id, url));
)");

const auto GAME_VIDEO_DDL = QLatin1String(R"(
CREATE TABLE IF NOT EXISTS game_video(
    game_id TEXT NOT NULL,
    provider TEXT NOT NULL,
    video_id TEXT NOT NULL,
    thumbnail_id TEXT NULL,
    name TEXT NULL,
    FOREIGN KEY (game_id) REFERENCES game (id) ON DELETE CASCADE,
    UNIQUE (game_id, provider, video_id));
)");

const auto GENRE_DDL = QLatin1String(R"(
CREATE TABLE IF NOT EXISTS genre(
    id TEXT PRIMARY KEY NOT NULL,
    name TEXT NOT NULL,
    slug TEXT NOT NULL);
)");

const auto OS_DDL = QLatin1String(R"(
CREATE TABLE IF NOT EXISTS os(
    slug TEXT PRIMARY KEY NOT NULL,
    name TEXT NOT NULL);
)");

const auto PUBLISHER_DDL = QLatin1String(R"(
CREATE TABLE IF NOT EXISTS publisher(
    id TEXT PRIMARY KEY NOT NULL,
    name TEXT NOT NULL,
    slug TEXT NOT NULL);
)");

const auto RELEASE_DDL = QLatin1String(R"(
CREATE TABLE IF NOT EXISTS release(
    id TEXT PRIMARY KEY NOT NULL,
    parent_id TEXT NULL,
    game_id TEXT NOT NULL,
    first_release_date NOT NULL,
    title TEXT NOT NULL,
    title_sort TEXT NOT NULL,
    type TEXT NOT NULL,
    summary TEXT NULL,
    logo TEXT NULL,
    FOREIGN KEY (game_id) REFERENCES game (id) ON DELETE CASCADE);
)");

const auto RELEASE_VIDEO_DDL = QLatin1String(R"(
CREATE TABLE IF NOT EXISTS release_video(
    release_id TEXT NOT NULL,
    provider TEXT NOT NULL,
    video_id TEXT NOT NULL,
    thumbnail_id TEXT NULL,
    name TEXT NULL,
    FOREIGN KEY (release_id) REFERENCES release (id) ON DELETE CASCADE,
    UNIQUE (release_id, provider, video_id));
)");

const auto THEME_DDL = QLatin1String(R"(
CREATE TABLE IF NOT EXISTS theme(
    id TEXT PRIMARY KEY NOT NULL,
    name TEXT NOT NULL,
    slug TEXT NOT NULL);
)");

// GamesDB entities' relations
const auto GAME_DEVELOPER_DDL = QLatin1String(R"(
CREATE TABLE IF NOT EXISTS game_developer(
    game_id TEXT NOT NULL,
    developer_id TEXT NOT NULL,
    FOREIGN KEY (game_id) REFERENCES game (id) ON DELETE CASCADE,
    FOREIGN KEY (developer_id) REFERENCES developer (id) ON DELETE CASCADE,
    UNIQUE (game_id, developer_id));
)");

const auto GAME_DLC_DDL = QLatin1String(R"(
CREATE TABLE IF NOT EXISTS game_dlc(
    game_id TEXT NOT NULL,
    dlc_id TEXT NOT NULL,
    FOREIGN KEY (game_id) REFERENCES game (id) ON DELETE CASCADE,
    UNIQUE (game_id, dlc_id));
)");

const auto GAME_GAME_MODE_DDL = QLatin1String(R"(
CREATE TABLE IF NOT EXISTS game_game_mode(
    game_id TEXT NOT NULL,
    game_mode_id TEXT NOT NULL,
    FOREIGN KEY (game_id) REFERENCES game (id) ON DELETE CASCADE,
    FOREIGN KEY (game_mode_id) REFERENCES game_mode (id) ON DELETE CASCADE,
    CONSTRAINT 'UK_game_game_mode_key' UNIQUE (game_id, game_mode_id));
)");

const auto GAME_GENRE_DDL = QLatin1String(R"(
CREATE TABLE IF NOT EXISTS game_genre(
    game_id TEXT NOT NULL,
    genre_id TEXT NOT NULL,
    FOREIGN KEY (game_id) REFERENCES game (id) ON DELETE CASCADE,
    FOREIGN KEY (genre_id) REFERENCES genre (id) ON DELETE CASCADE,
    UNIQUE (game_id, genre_id));
)");

const auto GAME_PUBLISHER_DDL = QLatin1String(R"(
CREATE TABLE IF NOT EXISTS game_publisher(
    game_id TEXT NOT NULL,
    publisher_id TEXT NOT NULL,
    FOREIGN KEY (game_id) REFERENCES game (id) ON DELETE CASCADE,
    FOREIGN KEY (publisher_id) REFERENCES publisher (id) ON DELETE CASCADE,
    UNIQUE (game_id, publisher_id));
)");

const auto GAME_THEME_DDL = QLatin1String(R"(
CREATE TABLE IF NOT EXISTS game_theme(
    game_id TEXT NOT NULL,
    theme_id TEXT NOT NULL,
    FOREIGN KEY (game_id) REFERENCES game (id) ON DELETE CASCADE,
    FOREIGN KEY (theme_id) REFERENCES theme (id) ON DELETE CASCADE,
    UNIQUE (game_id, theme_id));
)");

const auto RELEASE_AVAILABLE_LANGUAGE_DDL = QLatin1String(R"(
CREATE TABLE IF NOT EXISTS release_available_language(
    release_id TEXT NOT NULL,
    locale_code TEXT NOT NULL,
    FOREIGN KEY (release_id) REFERENCES release (id) ON DELETE CASCADE,
    UNIQUE (release_id, locale_code));
)");

const auto RELEASE_DLC_DDL = QLatin1String(R"(
CREATE TABLE IF NOT EXISTS release_dlc(
    release_id TEXT NOT NULL,
    dlc_id TEXT NOT NULL,
    FOREIGN KEY (release_id) REFERENCES release (id) ON DELETE CASCADE,
    UNIQUE (release_id, dlc_id));
)");

const auto RELEASE_GAME_MODE_DDL = QLatin1String(R"(
CREATE TABLE IF NOT EXISTS release_game_mode(
    release_id TEXT NOT NULL,
    game_mode_id TEXT NOT NULL,
    FOREIGN KEY (release_id) REFERENCES release (id) ON DELETE CASCADE,
    FOREIGN KEY (game_mode_id) REFERENCES game_mode (id) ON DELETE CASCADE,
    UNIQUE (release_id, game_mode_id));
)");

const auto RELEASE_SUPPORTED_OS_DDL = QLatin1String(R"(
CREATE TABLE IF NOT EXISTS release_supported_os(
    release_id TEXT NOT NULL,
    os_slug TEXT NOT NULL,
    FOREIGN KEY (release_id) REFERENCES release (id) ON DELETE CASCADE,
    FOREIGN KEY (os_slug) REFERENCES os (slug) ON DELETE CASCADE,
    UNIQUE (release_id, os_slug));
)");

// Mapping & technical info
const auto PLATFORM_RELEASE_DDL = QLatin1String(R"(
CREATE TABLE IF NOT EXISTS platform_release(
    platform TEXT NOT NULL,
    platform_release_id TEXT NOT NULL,
    release_id TEXT NULL,
    last_updated_at NOT NULL,
    FOREIGN KEY (release_id) REFERENCES release (id) ON DELETE CASCADE,
    UNIQUE (platform, platform_release_id))
)");

const auto PLATFORM_RELEASE_LAST_ACHIEVEMENTS_UPDATE_DDL = QLatin1String(R"(
CREATE TABLE IF NOT EXISTS platform_release_last_achievements_update(
    platform TEXT NOT NULL,
    platform_release_id TEXT NOT NULL,
    last_updated_at NOT NULL,
    UNIQUE (platform, platform_release_id))
)");


// User library
const auto USER_RELEASE_DDL = QLatin1String(R"(
CREATE TABLE IF NOT EXISTS user_release(
    user_id TEXT NOT NULL,
    platform TEXT NOT NULL,
    platform_release_id TEXT NOT NULL,
    created_at NOT NULL,
    owned_since NULL,
    owned BOOLEAN NOT NULL,
    hidden BOOLEAN NULL,
    rating INTEGER NULL,
    PRIMARY KEY (user_id, platform, platform_release_id));
)");

const auto USER_RELEASE_TAG_DDL = QLatin1String(R"(
CREATE TABLE IF NOT EXISTS user_release_tag(
    user_id TEXT NOT NULL,
    platform TEXT NOT NULL,
    platform_release_id TEXT NOT NULL,
    tag TEXT NOT NULL,
    CONSTRAINT 'CK_user_release_tag'
        CHECK (trim([tag]) <> ''),
    FOREIGN KEY (user_id, platform, platform_release_id) REFERENCES user_release (user_id, platform, platform_release_id) ON DELETE CASCADE,
    UNIQUE (user_id, platform, platform_release_id, tag));
)");

// QUERIES
const auto DELETE_GAME_ARTWORKS = QLatin1String(R"(
DELETE FROM game_artwork WHERE game_id = ?
)");

const auto DELETE_GAME_DEVELOPERS = QLatin1String(R"(
DELETE FROM game_developer WHERE game_id = ?
)");

const auto DELETE_GAME_DLCS = QLatin1String(R"(
DELETE FROM game_dlc WHERE game_id = ?
)");

const auto DELETE_GAME_GAME_MODES = QLatin1String(R"(
DELETE FROM game_game_mode WHERE game_id = ?
)");

const auto DELETE_GAME_GENRES = QLatin1String(R"(
DELETE FROM game_genre WHERE game_id = ?
)");

const auto DELETE_GAME_PUBLISHERS = QLatin1String(R"(
DELETE FROM game_publisher WHERE game_id = ?
)");

const auto DELETE_GAME_SCREENSHOTS = QLatin1String(R"(
DELETE FROM game_screenshot WHERE game_id = ?
)");

const auto DELETE_GAME_THEMES = QLatin1String(R"(
DELETE FROM game_theme WHERE game_id = ?
)");

const auto DELETE_GAME_VIDEOS = QLatin1String(R"(
DELETE FROM game_video WHERE game_id = ?
)");

const auto DELETE_RELEASE_AVAILABLE_LANGUAGES = QLatin1String(R"(
DELETE FROM release_available_language WHERE release_id = ?
)");

const auto DELETE_RELEASE_DLCS = QLatin1String(R"(
DELETE FROM release_dlc WHERE release_id = ?
)");

const auto DELETE_RELEASE_GAME_MODES = QLatin1String(R"(
DELETE FROM release_game_mode WHERE release_id = ?
)");

const auto DELETE_RELEASE_SUPPORTED_OS = QLatin1String(R"(
DELETE FROM release_supported_os WHERE release_id = ?
)");

const auto DELETE_RELEASE_VIDEOS = QLatin1String(R"(
DELETE FROM release_video WHERE release_id = ?
)");

const auto INSERT_DEVELOPER = QLatin1String(R"(
INSERT INTO developer(id, name, slug)
    VALUES(?, ?, ?)
    ON CONFLICT(id) DO UPDATE
    SET name = excluded.name, slug = excluded.slug;
)");

const auto INSERT_GAME = QLatin1String(R"(
INSERT INTO game(id, parent_id, first_release_date, title, title_sort, type, summary, visible, aggregated_rating, horizontal_artwork, background, vertical_cover, cover, logo, square_icon)
    VALUES(?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
    ON CONFLICT(id) DO UPDATE
    SET parent_id = excluded.parent_id, game_id = excluded.game_id, first_release_date = excluded.first_release_date,
        title = excluded.title, title_sort = excluded.title_sort, type = excluded.type,
        summary = excluded.summary, visible = excluded.visible, aggregated_rating = excluded.aggregated_rating,
        horizontal_artwork = excluded.horizontal_artwork, background = excluded.background, vertical_cover = excluded.vertical_cover, cover = excluded.cover,
        logo = excluded.logo, square_icon = excluded.square_icon;
)");

const auto INSERT_GAME_ARTWORK = QLatin1String(R"(
INSERT INTO game_artwork(game_id, url)
    VALUES(?, ?)
    ON CONFLICT(game_id, url) DO NOTHING;
)");

const auto INSERT_GAME_DEVELOPER = QLatin1String(R"(
INSERT INTO game_developer(game_id, developer_id)
    VALUES(?, ?)
    ON CONFLICT(game_id, developer_id) DO NOTHING;
)");

const auto INSERT_GAME_DLC = QLatin1String(R"(
INSERT INTO game_dlc(game_id, dlc_id)
    VALUES(?, ?)
    ON CONFLICT(game_id, dlc_id) DO NOTHING;
)");

const auto INSERT_GAME_GAME_MODE = QLatin1String(R"(
INSERT INTO game_game_mode(game_id, game_mode_id)
    VALUES(?, ?)
    ON CONFLICT(game_id, game_mode_id) DO NOTHING;
)");

const auto INSERT_GAME_GENRE = QLatin1String(R"(
INSERT INTO game_genre(game_id, genre_id)
    VALUES(?, ?)
    ON CONFLICT(game_id, genre_id) DO NOTHING;
)");

const auto INSERT_GAME_MODE = QLatin1String(R"(
INSERT INTO game_mode(id, name, slug)
    VALUES(?, ?, ?)
    ON CONFLICT(id) DO UPDATE
    SET name = excluded.name, slug = excluded.slug;
)");

const auto INSERT_GAME_PUBLISHER = QLatin1String(R"(
INSERT INTO game_publisher(game_id, publisher_id)
    VALUES(?, ?)
    ON CONFLICT(game_id, publisher_id) DO NOTHING;
)");

const auto INSERT_GAME_SCREENSHOT = QLatin1String(R"(
INSERT INTO game_screenshot(game_id, url)
    VALUES(?, ?)
    ON CONFLICT(game_id, url) DO NOTHING;
)");

const auto INSERT_GAME_THEME = QLatin1String(R"(
INSERT INTO game_theme(game_id, theme_id)
    VALUES(?, ?)
    ON CONFLICT(game_id, theme_id) DO NOTHING;
)");

const auto INSERT_GAME_VIDEO = QLatin1String(R"(
INSERT INTO game_video(game_id, provider, video_id, thumbnail_id, name)
    VALUES(?, ?, ?, ?, ?)
    ON CONFLICT(game_id, provider, video_id) DO UPDATE
    SET thumbnail_id = excluded.thumbnail_id, name = excluded.name;
)");

const auto INSERT_GENRE = QLatin1String(R"(
INSERT INTO genre(id, name, slug)
    VALUES(?, ?, ?)
    ON CONFLICT(id) DO UPDATE
    SET name = excluded.name, slug = excluded.slug;
)");

const auto INSERT_OS = QLatin1String(R"(
INSERT INTO os(slug, name)
    VALUES(?, ?)
    ON CONFLICT(slug) DO UPDATE
    SET name = excluded.name;
)");

const auto INSERT_PLATFORM_RELEASE = QLatin1String(R"(
INSERT INTO platform_release(platform, platform_release_id, release_id, last_updated_at)
    VALUES(?, ?, ?, ?)
    ON CONFLICT(platform, platform_release_id) DO UPDATE
    SET release_id = excluded.release_id, last_updated_at = excluded.last_updated_at;
)");

const auto INSERT_PUBLISHER = QLatin1String(R"(
INSERT INTO publisher(id, name, slug)
    VALUES(?, ?, ?)
    ON CONFLICT(id) DO UPDATE
    SET name = excluded.name, slug = excluded.slug;
)");

const auto INSERT_RELEASE = QLatin1String(R"(
INSERT INTO release(id, parent_id, game_id, first_release_date, title, title_sort, type, summary, logo)
    VALUES(?, ?, ?, ?, ?, ?, ?, ?, ?)
    ON CONFLICT(id) DO UPDATE
    SET parent_id = excluded.parent_id, game_id = excluded.game_id, first_release_date = excluded.first_release_date,
        title = excluded.title, title_sort = excluded.title_sort, type = excluded.type, summary = excluded.summary, logo = excluded.logo;
)");

const auto INSERT_RELEASE_AVAILABLE_LANGUAGE = QLatin1String(R"(
INSERT INTO release_available_language(release_id, locale_code)
    VALUES(?, ?)
    ON CONFLICT(release_id, locale_code) DO NOTHING;
)");

const auto INSERT_RELEASE_DLC = QLatin1String(R"(
INSERT INTO release_dlc(release_id, dlc_id)
    VALUES(?, ?)
    ON CONFLICT(release_id, dlc_id) DO NOTHING;
)");

const auto INSERT_RELEASE_GAME_MODE = QLatin1String(R"(
INSERT INTO release_game_mode(release_id, game_mode_id)
    VALUES(?, ?)
    ON CONFLICT(release_id, game_mode_id) DO NOTHING;
)");

const auto INSERT_RELEASE_SUPPORTED_OS = QLatin1String(R"(
INSERT INTO release_supported_os(release_id, os_slug)
    VALUES(?, ?)
    ON CONFLICT(release_id, os_slug) DO NOTHING;
)");

const auto INSERT_RELEASE_VIDEO = QLatin1String(R"(
INSERT INTO release_video(release_id, provider, video_id, thumbnail_id, name)
    VALUES(?, ?, ?, ?, ?)
    ON CONFLICT(release_id, provider, video_id) DO UPDATE
    SET thumbnail_id = excluded.thumbnail_id, name = excluded.name;
)");

const auto INSERT_THEME = QLatin1String(R"(
INSERT INTO theme(id, name, slug)
    VALUES(?, ?, ?)
    ON CONFLICT(id) DO UPDATE
    SET name = excluded.name, slug = excluded.slug;
)");

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

void db::initialize()
{
    QDir dataDirectory(QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation));
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(dataDirectory.filePath("library.db"));

    if (!db.open())
    {
        qDebug() << "Could not open database: " << db.lastError();
        exit(-1);
    }

    QSqlQuery initializationQuery;
    // Creating tables to store user library info
    if (!initializationQuery.exec(USER_RELEASE_DDL))
    {
        qDebug() << "Could not create user release table: " << initializationQuery.lastError();
        exit(-1);
    }
    if (!initializationQuery.exec(USER_RELEASE_TAG_DDL))
    {
        qDebug() << "Could not create user release tags table: " << initializationQuery.lastError();
        exit(-1);
    }

    // Creating tables for inter-platform mappings & other technical info
    if (!initializationQuery.exec(PLATFORM_RELEASE_DDL))
    {
        qDebug() << "Could not create platform release table: " << initializationQuery.lastError();
        exit(-1);
    }
    if (!initializationQuery.exec(PLATFORM_RELEASE_LAST_ACHIEVEMENTS_UPDATE_DDL))
    {
        qDebug() << "Could not create platform release last achievements update table: " << initializationQuery.lastError();
        exit(-1);
    }

    // Creating tables to store game database data
    if (!initializationQuery.exec(DEVELOPER_DDL))
    {
        qDebug() << "Could not create developer table: " << initializationQuery.lastError();
        exit(-1);
    }
    if (!initializationQuery.exec(GAME_DDL))
    {
        qDebug() << "Could not create game table: " << initializationQuery.lastError();
        exit(-1);
    }
    if (!initializationQuery.exec(GAME_ARTWORK_DDL))
    {
        qDebug() << "Could not create game artwork table: " << initializationQuery.lastError();
        exit(-1);
    }
    if (!initializationQuery.exec(GAME_MODE_DDL))
    {
        qDebug() << "Could not create game mode table: " << initializationQuery.lastError();
        exit(-1);
    }
    if (!initializationQuery.exec(GAME_SCREENSHOT_DDL))
    {
        qDebug() << "Could not create game screenshot table: " << initializationQuery.lastError();
        exit(-1);
    }
    if (!initializationQuery.exec(GAME_VIDEO_DDL))
    {
        qDebug() << "Could not create game video table: " << initializationQuery.lastError();
        exit(-1);
    }
    if (!initializationQuery.exec(GENRE_DDL))
    {
        qDebug() << "Could not create achievement genre table: " << initializationQuery.lastError();
        exit(-1);
    }
    if (!initializationQuery.exec(OS_DDL))
    {
        qDebug() << "Could not create OS table: " << initializationQuery.lastError();
        exit(-1);
    }
    if (!initializationQuery.exec(PUBLISHER_DDL))
    {
        qDebug() << "Could not create publisher table: " << initializationQuery.lastError();
        exit(-1);
    }
    if (!initializationQuery.exec(RELEASE_DDL))
    {
        qDebug() << "Could not create release table: " << initializationQuery.lastError();
        exit(-1);
    }
    if (!initializationQuery.exec(RELEASE_VIDEO_DDL))
    {
        qDebug() << "Could not create release video table: " << initializationQuery.lastError();
        exit(-1);
    }
    if (!initializationQuery.exec(THEME_DDL))
    {
        qDebug() << "Could not create theme table: " << initializationQuery.lastError();
        exit(-1);
    }

    if (!initializationQuery.exec(GAME_DEVELOPER_DDL))
    {
        qDebug() << "Could not create game develoer table: " << initializationQuery.lastError();
        exit(-1);
    }
    if (!initializationQuery.exec(GAME_DLC_DDL))
    {
        qDebug() << "Could not create game dl table: " << initializationQuery.lastError();
        exit(-1);
    }
    if (!initializationQuery.exec(GAME_GAME_MODE_DDL))
    {
        qDebug() << "Could not create game supported game mode table: " << initializationQuery.lastError();
        exit(-1);
    }
    if (!initializationQuery.exec(GAME_GENRE_DDL))
    {
        qDebug() << "Could not create game genre table: " << initializationQuery.lastError();
        exit(-1);
    }
    if (!initializationQuery.exec(GAME_PUBLISHER_DDL))
    {
        qDebug() << "Could not create game publisher table: " << initializationQuery.lastError();
        exit(-1);
    }
    if (!initializationQuery.exec(GAME_THEME_DDL))
    {
        qDebug() << "Could not create game theme table: " << initializationQuery.lastError();
        exit(-1);
    }
    if (!initializationQuery.exec(RELEASE_AVAILABLE_LANGUAGE_DDL))
    {
        qDebug() << "Could not create release available language table: " << initializationQuery.lastError();
        exit(-1);
    }
    if (!initializationQuery.exec(RELEASE_DLC_DDL))
    {
        qDebug() << "Could not create release dlc table: " << initializationQuery.lastError();
        exit(-1);
    }
    if (!initializationQuery.exec(RELEASE_GAME_MODE_DDL))
    {
        qDebug() << "Could not create release supported game mode table: " << initializationQuery.lastError();
        exit(-1);
    }
    if (!initializationQuery.exec(RELEASE_SUPPORTED_OS_DDL))
    {
        qDebug() << "Could not create release supported OS table: " << initializationQuery.lastError();
        exit(-1);
    }

    // Creating tables to store gameplay info & statistics
    if (!initializationQuery.exec(ACHIEVEMENT_DDL))
    {
        qDebug() << "Could not create achievement table: " << initializationQuery.lastError();
        exit(-1);
    }
    if (!initializationQuery.exec(ACHIEVEMENT_LOCALIZED_DESCRIPTION_DDL))
    {
        qDebug() << "Could not create achievement localized description table: " << initializationQuery.lastError();
        exit(-1);
    }
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

bool saveLinkedIdMetatag(const QString &id, const QVector<api::IdMetaTag> &metatags,
                         const QString &deleteQueryText, const QString &insertMetatagsQueryText, const QString &inserLinksQueryText)
{
    QSqlQuery deleteStaleLinksQuery;
    if (!deleteStaleLinksQuery.prepare(deleteQueryText))
    {
        return false;
    }
    deleteStaleLinksQuery.addBindValue(id);
    if (!deleteStaleLinksQuery.exec())
    {
        return false;
    }
    if (!metatags.isEmpty())
    {
        QVariantList ids(metatags.count(), id);
        QVariantList metatagsIds(metatags.count());
        QVariantList metatagsNames(metatags.count());
        QVariantList metatagsSlugs(metatags.count());
        for (const api::IdMetaTag &metatag : std::as_const(metatags))
        {
            metatagsIds << metatag.id;
            metatagsNames << metatag.name;
            metatagsSlugs << metatag.slug;
        }

        QSqlQuery insertMetatagsQuery;
        if (!insertMetatagsQuery.prepare(insertMetatagsQueryText))
        {
            return false;
        }
        insertMetatagsQuery.addBindValue(metatagsIds);
        insertMetatagsQuery.addBindValue(metatagsNames);
        insertMetatagsQuery.addBindValue(metatagsSlugs);
        if (!insertMetatagsQuery.exec())
        {
            return false;
        }

        QSqlQuery insertLinksQuery;
        if (!insertLinksQuery.prepare(inserLinksQueryText))
        {
            return false;
        }
        insertLinksQuery.addBindValue(ids);
        insertLinksQuery.addBindValue(metatagsIds);
        if (!insertLinksQuery.exec())
        {
            return false;
        }
    }

    return true;
}

bool saveLinkedLocalizedMetatag(const QString &id, const QVector<api::LocalizedMetaTag> &metatags,
                                const QString &deleteLinksQueryText, const QString &insertMetatagsQueryText, const QString &inserLinksQueryText)
{
    QSqlQuery deleteStaleLinksQuery;
    if (!deleteStaleLinksQuery.prepare(deleteLinksQueryText))
    {
        return false;
    }
    deleteStaleLinksQuery.addBindValue(id);
    if (!deleteStaleLinksQuery.exec())
    {
        return false;
    }
    if (!metatags.isEmpty())
    {
        QVariantList ids(metatags.count(), id);
        QVariantList metatagsIds(metatags.count());
        QVariantList metatagsNames(metatags.count());
        QVariantList metatagsSlugs(metatags.count());
        for (const api::LocalizedMetaTag &metatag : std::as_const(metatags))
        {
            metatagsIds << metatag.id;
            // TODO: persist all of language-specific stuff
            metatagsNames << metatag.name["*"];
            metatagsSlugs << metatag.slug;
        }

        QSqlQuery insertMetatagsQuery;
        if (!insertMetatagsQuery.prepare(insertMetatagsQueryText))
        {
            return false;
        }
        insertMetatagsQuery.addBindValue(metatagsIds);
        insertMetatagsQuery.addBindValue(metatagsNames);
        insertMetatagsQuery.addBindValue(metatagsSlugs);
        if (!insertMetatagsQuery.exec())
        {
            return false;
        }

        QSqlQuery insertLinksQuery;
        if (!insertLinksQuery.prepare(inserLinksQueryText))
        {
            return false;
        }
        insertLinksQuery.addBindValue(ids);
        insertLinksQuery.addBindValue(metatagsIds);
        if (!insertLinksQuery.exec())
        {
            return false;
        }
    }

    return true;
}

bool saveLinkedValues(const QString &id, const QVector<QString> &values,
                      const QString &deleteQueryText, const QString &insertQueryText)
{
    QSqlQuery deleteStaleLinksQuery;
    if (!deleteStaleLinksQuery.prepare(deleteQueryText))
    {
        return false;
    }
    deleteStaleLinksQuery.addBindValue(id);
    if (!deleteStaleLinksQuery.exec())
    {
        return false;
    }
    if (!values.isEmpty())
    {
        QVariantList ids(values.count(), id);
        QVariantList valuesVariants(values.count());
        for (const QString &value : std::as_const(values))
        {
            valuesVariants << value;
        }

        QSqlQuery insertValuesQuery;
        if (!insertValuesQuery.prepare(insertQueryText))
        {
            return false;
        }
        insertValuesQuery.addBindValue(ids);
        insertValuesQuery.addBindValue(valuesVariants);
        if (!insertValuesQuery.exec())
        {
            return false;
        }
    }

    return true;
}

bool saveLinkedNamedVideos(const QString &id, const QVector<api::NamedVideo> &videos,
                           const QString &deleteQueryText, const QString &insertQueryText)
{

    QSqlQuery deleteStaleLinksQuery;
    if (!deleteStaleLinksQuery.prepare(deleteQueryText))
    {
        return false;
    }
    deleteStaleLinksQuery.addBindValue(id);
    if (!deleteStaleLinksQuery.exec())
    {
        return false;
    }
    if (!videos.isEmpty())
    {
        QVariantList ids(videos.count(), id);
        QVariantList videosProviders(videos.count());
        QVariantList videosIds(videos.count());
        QVariantList videosThumbnailIds(videos.count());
        QVariantList videosNames(videos.count());
        for (const api::NamedVideo &video : std::as_const(videos))
        {
            videosProviders << video.provider;
            videosIds << video.videoId;
            videosThumbnailIds << video.thumbnailId;
            videosNames << video.name;
        }

        QSqlQuery insertMetatagsQuery;
        if (!insertMetatagsQuery.prepare(insertQueryText))
        {
            return false;
        }
        insertMetatagsQuery.addBindValue(ids);
        insertMetatagsQuery.addBindValue(videosProviders);
        insertMetatagsQuery.addBindValue(videosIds);
        insertMetatagsQuery.addBindValue(videosThumbnailIds);
        insertMetatagsQuery.addBindValue(videosNames);
        if (!insertMetatagsQuery.exec())
        {
            return false;
        }
    }

    return true;
}

void db::saveRelease(const api::Release &release)
{
    QSqlDatabase db = QSqlDatabase::database();
    QSqlQuery insertPlatformReleaseQuery;
    if (!insertPlatformReleaseQuery.prepare(INSERT_PLATFORM_RELEASE))
    {
        qDebug() << "Failed to prepare query to save platform release";
        return;
    }
    insertPlatformReleaseQuery.addBindValue(release.platformId);
    insertPlatformReleaseQuery.addBindValue(release.externalId);
    insertPlatformReleaseQuery.addBindValue(release.id);
    insertPlatformReleaseQuery.addBindValue(QDateTime::currentDateTime());

    if (!db.transaction())
    {
        qDebug() << "Failed to start DB transaction";
        return;
    }

    if (!insertPlatformReleaseQuery.exec())
    {
        db.rollback();
        qDebug() << "Failed to persist platform release" << insertPlatformReleaseQuery.lastError();
        return;
    }
    if (release.id.isNull())
    {
        db.commit();
        return;
    }

    if (release.type != "game" && release.type != "dlc")
    {
        db.rollback();
        qDebug() << "Unknown release kind, aborting for data consistence. Kind: " << release.type << "Release: " << release.id;
        return;
    }

    QSqlQuery insertGameQuery;
    if (!insertGameQuery.prepare(INSERT_GAME))
    {
        db.rollback();
        qDebug() << "Failed to prepare query to persist game";
        return;
    }
    insertGameQuery.addBindValue(release.game.id);
    insertGameQuery.addBindValue(release.game.parentId);
    insertGameQuery.addBindValue(release.game.firstReleaseDate);
    insertGameQuery.addBindValue(release.game.title);
    insertGameQuery.addBindValue(release.game.sortingTitle);
    insertGameQuery.addBindValue(release.game.type);
    insertGameQuery.addBindValue(release.game.summary);
    insertGameQuery.addBindValue(release.game.visibleInLibrary);
    insertGameQuery.addBindValue(release.game.aggregatedRating);
    insertGameQuery.addBindValue(release.game.horizontalArtwork);
    insertGameQuery.addBindValue(release.game.background);
    insertGameQuery.addBindValue(release.game.verticalCover);
    insertGameQuery.addBindValue(release.game.cover);
    insertGameQuery.addBindValue(release.game.logo);
    insertGameQuery.addBindValue(release.game.squareIcon);
    if (!insertGameQuery.exec())
    {
        db.rollback();
        qDebug() << "Failed to persist game" << insertGameQuery.lastError();
        return;
    }

    if (!saveLinkedValues(release.game.id, release.game.dlcsIds, DELETE_GAME_DLCS, INSERT_GAME_DLC))
    {
        db.rollback();
        qDebug() << "Failed to persist game DLCs";
        return;
    }
    if (!saveLinkedIdMetatag(release.game.id, release.game.developers,
                             DELETE_GAME_DEVELOPERS, INSERT_DEVELOPER, INSERT_GAME_DEVELOPER))
    {
        db.rollback();
        qDebug() << "Failed to persist game developers";
        return;
    }
    if (!saveLinkedIdMetatag(release.game.id, release.game.publishers,
                             DELETE_GAME_PUBLISHERS, INSERT_PUBLISHER, INSERT_GAME_PUBLISHER))
    {
        db.rollback();
        qDebug() << "Failed to persist game publishers";
        return;
    }
    if (!saveLinkedLocalizedMetatag(release.game.id, release.game.genres,
                             DELETE_GAME_GENRES, INSERT_GENRE, INSERT_GAME_GENRE))
    {
        db.rollback();
        qDebug() << "Failed to persist game genres";
        return;
    }
    if (!saveLinkedLocalizedMetatag(release.game.id, release.game.themes,
                             DELETE_GAME_THEMES, INSERT_THEME, INSERT_GAME_THEME))
    {
        db.rollback();
        qDebug() << "Failed to persist game themes";
        return;
    }
    if (!saveLinkedValues(release.game.id, release.game.screenshots, DELETE_GAME_SCREENSHOTS, INSERT_GAME_SCREENSHOT))
    {
        db.rollback();
        qDebug() << "Failed to persist game screenshots";
        return;
    }
    if (!saveLinkedNamedVideos(release.game.id, release.game.videos, DELETE_GAME_VIDEOS, INSERT_GAME_VIDEO))
    {
        db.rollback();
        qDebug() << "Failed to persist game videos";
        return;
    }
    if (!saveLinkedValues(release.game.id, release.game.screenshots, DELETE_GAME_ARTWORKS, INSERT_GAME_ARTWORK))
    {
        db.rollback();
        qDebug() << "Failed to persist game artworks";
        return;
    }
    if (!saveLinkedIdMetatag(release.game.id, release.game.gameModes,
                             DELETE_GAME_GAME_MODES, INSERT_GAME_MODE, INSERT_GAME_GAME_MODE))
    {
        db.rollback();
        qDebug() << "Failed to persist game supported game modes";
        return;
    }

    QSqlQuery insertReleaseQuery;
    if (!insertReleaseQuery.prepare(INSERT_RELEASE))
    {
        db.rollback();
        qDebug() << "Failed to prepare query to persist release";
        return;
    }
    insertReleaseQuery.addBindValue(release.id);
    insertReleaseQuery.addBindValue(release.parentId);
    insertReleaseQuery.addBindValue(release.gameId);
    insertReleaseQuery.addBindValue(release.firstReleaseDate);
    insertReleaseQuery.addBindValue(release.title);
    insertReleaseQuery.addBindValue(release.sortingTitle);
    insertReleaseQuery.addBindValue(release.type);
    insertReleaseQuery.addBindValue(release.summary);
    insertReleaseQuery.addBindValue(release.logo);
    if (!insertReleaseQuery.exec())
    {
        db.rollback();
        qDebug() << "Failed to persist release" << insertGameQuery.lastError();
        return;
    }

    QSqlQuery deleteStaleOsLinksQuery;
    if (!deleteStaleOsLinksQuery.prepare(DELETE_RELEASE_SUPPORTED_OS))
    {
        db.rollback();
        qDebug() << "Failed to prepare release OS cleanup";
        return;
    }
    deleteStaleOsLinksQuery.addBindValue(release.id);
    if (!deleteStaleOsLinksQuery.exec())
    {
        db.rollback();
        qDebug() << "Failed to execute release OS cleanup";
        return;
    }
    if (!release.supportedOperatingSystems.isEmpty())
    {
        QVariantList ids(release.supportedOperatingSystems.count(), release.id);
        QVariantList metatagsNames(release.supportedOperatingSystems.count());
        QVariantList metatagsSlugs(release.supportedOperatingSystems.count());
        for (const api::MetaTag &metatag : std::as_const(release.supportedOperatingSystems))
        {
            metatagsNames << metatag.name;
            metatagsSlugs << metatag.slug;
        }

        QSqlQuery insertMetatagsQuery;
        if (!insertMetatagsQuery.prepare(INSERT_OS))
        {
            db.rollback();
            qDebug() << "Failed to prepare OS persistence";
            return;
        }
        insertMetatagsQuery.addBindValue(metatagsSlugs);
        insertMetatagsQuery.addBindValue(metatagsNames);
        if (!insertMetatagsQuery.exec())
        {
            db.rollback();
            qDebug() << "Failed to persist OS";
            return;
        }

        QSqlQuery insertLinksQuery;
        if (!insertLinksQuery.prepare(INSERT_RELEASE_SUPPORTED_OS))
        {
            db.rollback();
            qDebug() << "Failed to prepare release OS persistence";
            return;
        }
        insertLinksQuery.addBindValue(ids);
        insertLinksQuery.addBindValue(metatagsSlugs);
        if (!insertLinksQuery.exec())
        {
            db.rollback();
            qDebug() << "Failed to persist release OS";
            return;
        }
    }

    if (!saveLinkedValues(release.id, release.availableLanguages, DELETE_RELEASE_AVAILABLE_LANGUAGES, INSERT_RELEASE_AVAILABLE_LANGUAGE))
    {
        db.rollback();
        qDebug() << "Failed to persist release available languages";
        return;
    }
    if (!saveLinkedValues(release.id, release.dlcsIds, DELETE_RELEASE_DLCS, INSERT_RELEASE_DLC))
    {
        db.rollback();
        qDebug() << "Failed to persist release DLCs";
        return;
    }
    if (!saveLinkedNamedVideos(release.id, release.videos, DELETE_RELEASE_VIDEOS, INSERT_RELEASE_VIDEO))
    {
        db.rollback();
        qDebug() << "Failed to persist release videos";
        return;
    }
    if (!saveLinkedIdMetatag(release.id, release.gameModes,
                             DELETE_RELEASE_GAME_MODES, INSERT_GAME_MODE, INSERT_RELEASE_GAME_MODE))
    {
        db.rollback();
        qDebug() << "Failed to persist release supported game modes";
        return;
    }

    if (!db.commit())
    {
        db.rollback();
        qDebug() << "Failed to complete release save";
        return;
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
