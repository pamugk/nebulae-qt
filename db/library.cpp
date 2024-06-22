 #include "database.h"

#include <QDebug>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>

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
    SET parent_id = excluded.parent_id, first_release_date = excluded.first_release_date,
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

bool saveLinkedIdMetatag(const QString &id, const QVector<api::IdMetaTag> &metatags,
                         const QString &deleteQueryText, const QString &insertMetatagsQueryText, const QString &inserLinksQueryText)
{
    QSqlQuery deleteStaleLinksQuery;
    if (!deleteStaleLinksQuery.prepare(deleteQueryText))
    {
        qDebug() << deleteStaleLinksQuery.lastError();
        return false;
    }
    deleteStaleLinksQuery.addBindValue(id);
    if (!deleteStaleLinksQuery.exec())
    {
        qDebug() << deleteStaleLinksQuery.lastError();
        return false;
    }
    if (!metatags.isEmpty())
    {
        QVariantList ids(metatags.count(), id);
        QVariantList metatagsIds(metatags.count());
        QVariantList metatagsNames(metatags.count());
        QVariantList metatagsSlugs(metatags.count());
        for (std::size_t i = 0; i < metatags.count(); i++)
        {
            const auto &metatag = metatags[i];
            metatagsIds[i] = metatag.id;
            metatagsNames[i] = metatag.name;
            metatagsSlugs[i] = metatag.slug;
        }

        QSqlQuery insertMetatagsQuery;
        if (!insertMetatagsQuery.prepare(insertMetatagsQueryText))
        {
            qDebug() << insertMetatagsQuery.lastError();
            return false;
        }
        insertMetatagsQuery.addBindValue(metatagsIds);
        insertMetatagsQuery.addBindValue(metatagsNames);
        insertMetatagsQuery.addBindValue(metatagsSlugs);
        if (!insertMetatagsQuery.execBatch())
        {
            qDebug() << insertMetatagsQuery.lastError();
            return false;
        }

        QSqlQuery insertLinksQuery;
        if (!insertLinksQuery.prepare(inserLinksQueryText))
        {
            qDebug() << insertLinksQuery.lastError();
            return false;
        }
        insertLinksQuery.addBindValue(ids);
        insertLinksQuery.addBindValue(metatagsIds);
        if (!insertLinksQuery.execBatch())
        {
            qDebug() << insertLinksQuery.lastError();
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
        qDebug() << deleteStaleLinksQuery.lastError();
        return false;
    }
    deleteStaleLinksQuery.addBindValue(id);
    if (!deleteStaleLinksQuery.exec())
    {
        qDebug() << deleteStaleLinksQuery.lastError();
        return false;
    }
    if (!metatags.isEmpty())
    {
        QVariantList ids(metatags.count(), id);
        QVariantList metatagsIds(metatags.count());
        QVariantList metatagsNames(metatags.count());
        QVariantList metatagsSlugs(metatags.count());
        for (std::size_t i = 0; i < metatags.count(); i++)
        {
            const auto &metatag = metatags[i];
            metatagsIds[i] = metatag.id;
            // TODO: persist all of language-specific stuff
            metatagsNames[i] = metatag.name["*"];
            metatagsSlugs[i] = metatag.slug;
        }

        QSqlQuery insertMetatagsQuery;
        if (!insertMetatagsQuery.prepare(insertMetatagsQueryText))
        {
            qDebug() << insertMetatagsQuery.lastError();
            return false;
        }
        insertMetatagsQuery.addBindValue(metatagsIds);
        insertMetatagsQuery.addBindValue(metatagsNames);
        insertMetatagsQuery.addBindValue(metatagsSlugs);
        if (!insertMetatagsQuery.execBatch())
        {
            qDebug() << insertMetatagsQuery.lastError();
            return false;
        }

        QSqlQuery insertLinksQuery;
        if (!insertLinksQuery.prepare(inserLinksQueryText))
        {
            qDebug() << insertLinksQuery.lastError();
            return false;
        }
        insertLinksQuery.addBindValue(ids);
        insertLinksQuery.addBindValue(metatagsIds);
        if (!insertLinksQuery.execBatch())
        {
            qDebug() << insertLinksQuery.lastError();
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
        qDebug() << deleteStaleLinksQuery.lastError();
        return false;
    }
    deleteStaleLinksQuery.addBindValue(id);
    if (!deleteStaleLinksQuery.exec())
    {
        qDebug() << deleteStaleLinksQuery.lastError();
        return false;
    }
    if (!values.isEmpty())
    {
        QVariantList ids(values.count(), id);
        QVariantList valuesVariants(values.count());
        for (std::size_t i = 0; i < values.count(); i++)
        {
            valuesVariants[i] = values[i];
        }

        QSqlQuery insertValuesQuery;
        if (!insertValuesQuery.prepare(insertQueryText))
        {
            qDebug() << insertValuesQuery.lastError();
            return false;
        }
        insertValuesQuery.addBindValue(ids);
        insertValuesQuery.addBindValue(valuesVariants);
        if (!insertValuesQuery.execBatch())
        {
            qDebug() << insertValuesQuery.lastError();
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
        qDebug() << deleteStaleLinksQuery.lastError();
        return false;
    }
    deleteStaleLinksQuery.addBindValue(id);
    if (!deleteStaleLinksQuery.exec())
    {
        qDebug() << deleteStaleLinksQuery.lastError();
        return false;
    }
    if (!videos.isEmpty())
    {
        QVariantList ids(videos.count(), id);
        QVariantList videosProviders(videos.count());
        QVariantList videosIds(videos.count());
        QVariantList videosThumbnailIds(videos.count());
        QVariantList videosNames(videos.count());
        for (std::size_t i = 0; i < videos.count(); i++)
        {
            const auto &video = videos[i];
            videosProviders[i] = video.provider;
            videosIds[i] = video.videoId;
            videosThumbnailIds[i] = video.thumbnailId;
            videosNames[i] = video.name;
        }

        QSqlQuery insertMetatagsQuery;
        if (!insertMetatagsQuery.prepare(insertQueryText))
        {
            qDebug() << insertMetatagsQuery.lastError();
            return false;
        }
        insertMetatagsQuery.addBindValue(ids);
        insertMetatagsQuery.addBindValue(videosProviders);
        insertMetatagsQuery.addBindValue(videosIds);
        insertMetatagsQuery.addBindValue(videosThumbnailIds);
        insertMetatagsQuery.addBindValue(videosNames);
        if (!insertMetatagsQuery.execBatch())
        {
            qDebug() << insertMetatagsQuery.lastError();
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
        qDebug() << "Failed to prepare query to save platform release" << insertPlatformReleaseQuery.lastError();
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
        qDebug() << "Failed to prepare query to persist game" << insertGameQuery.lastError();
        return;
    }
    insertGameQuery.addBindValue(release.game.id);
    insertGameQuery.addBindValue(release.game.parentId);
    insertGameQuery.addBindValue(release.game.firstReleaseDate);
    insertGameQuery.addBindValue(release.game.title["*"]);
    insertGameQuery.addBindValue(release.game.sortingTitle["*"]);
    insertGameQuery.addBindValue(release.game.type);
    insertGameQuery.addBindValue(release.game.summary["*"]);
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
        qDebug() << "Failed to prepare query to persist release" << insertReleaseQuery.lastError();
        return;
    }
    insertReleaseQuery.addBindValue(release.id);
    insertReleaseQuery.addBindValue(release.parentId);
    insertReleaseQuery.addBindValue(release.gameId);
    insertReleaseQuery.addBindValue(release.firstReleaseDate);
    insertReleaseQuery.addBindValue(release.title["*"]);
    insertReleaseQuery.addBindValue(release.sortingTitle["*"]);
    insertReleaseQuery.addBindValue(release.type);
    insertReleaseQuery.addBindValue(release.summary["*"]);
    insertReleaseQuery.addBindValue(release.logo);
    if (!insertReleaseQuery.exec())
    {
        db.rollback();
        qDebug() << "Failed to persist release" << insertReleaseQuery.lastError();
        return;
    }

    QSqlQuery deleteStaleOsLinksQuery;
    if (!deleteStaleOsLinksQuery.prepare(DELETE_RELEASE_SUPPORTED_OS))
    {
        db.rollback();
        qDebug() << "Failed to prepare release OS cleanup" << deleteStaleOsLinksQuery.lastError();
        return;
    }
    deleteStaleOsLinksQuery.addBindValue(release.id);
    if (!deleteStaleOsLinksQuery.exec())
    {
        db.rollback();
        qDebug() << "Failed to execute release OS cleanup" << deleteStaleOsLinksQuery.lastError();
        return;
    }
    if (!release.supportedOperatingSystems.isEmpty())
    {
        QVariantList ids(release.supportedOperatingSystems.count(), release.id);
        QVariantList metatagsNames(release.supportedOperatingSystems.count());
        QVariantList metatagsSlugs(release.supportedOperatingSystems.count());
        for (std::size_t i = 0; i < release.supportedOperatingSystems.count(); i++)
        {
            const auto &metatag = release.supportedOperatingSystems[i];
            metatagsNames[i] = metatag.name;
            metatagsSlugs[i] = metatag.slug;
        }

        QSqlQuery insertMetatagsQuery;
        if (!insertMetatagsQuery.prepare(INSERT_OS))
        {
            db.rollback();
            qDebug() << "Failed to prepare OS persistence" << insertMetatagsQuery.lastError();
            return;
        }
        insertMetatagsQuery.addBindValue(metatagsSlugs);
        insertMetatagsQuery.addBindValue(metatagsNames);
        if (!insertMetatagsQuery.execBatch())
        {
            db.rollback();
            qDebug() << "Failed to persist OS" << insertMetatagsQuery.lastError();
            return;
        }

        QSqlQuery insertLinksQuery;
        if (!insertLinksQuery.prepare(INSERT_RELEASE_SUPPORTED_OS))
        {
            db.rollback();
            qDebug() << "Failed to prepare release OS persistence" << insertLinksQuery.lastError();
            return;
        }
        insertLinksQuery.addBindValue(ids);
        insertLinksQuery.addBindValue(metatagsSlugs);
        if (!insertLinksQuery.execBatch())
        {
            db.rollback();
            qDebug() << "Failed to persist release OS" << insertLinksQuery.lastError();
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
