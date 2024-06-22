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
    first_release_date NULL,
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
    first_release_date NULL,
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
