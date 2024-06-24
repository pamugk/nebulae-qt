#include "gameserialization.h"

#include <QJsonArray>
#include <QJsonObject>

void parseGame(const QJsonValue &json, api::Game &data)
{
    data.id = json["id"].toString();
    data.parentId = json["parent_id"].toString();
    auto dlcsIds = json["dlcs_ids"].toArray();
    data.dlcsIds.reserve(dlcsIds.count());
    for (const QJsonValue &dlcId : std::as_const(dlcsIds))
    {
        data.dlcsIds << dlcId.toString();
    }
    data.firstReleaseDate = QDateTime::fromString(json["first_release_date"].toString(), Qt::ISODate);
    auto releases = json["releases"].toArray();
    data.releases.resize(releases.count());
    for (std::size_t i = 0; i < releases.count(); i++)
    {
        parsePlatformRelease(releases[i], data.releases[i]);
    }
    data.title = json["title"].toObject().toVariantMap();
    data.sortingTitle = json["sorting_title"].toObject().toVariantMap();
    data.type = json["type"].toString();
    auto developersIds = json["developers_ids"].toArray();
    data.developersIds.reserve(developersIds.count());
    for (const QJsonValue &developerId : std::as_const(developersIds))
    {
        data.developersIds << developerId.toString();
    }
    auto developers = json["developers"].toArray();
    data.developers.resize(developers.count());
    for (std::size_t i = 0; i < developers.count(); i++)
    {
        parseIdMetaTag(developers[i], data.developers[i]);
    }
    auto publishersIds = json["publishers_ids"].toArray();
    data.publishersIds.reserve(publishersIds.count());
    for (const QJsonValue &publisherId : std::as_const(publishersIds))
    {
        data.publishersIds << publisherId.toString();
    }
    auto publishers = json["publishers"].toArray();
    data.publishers.resize(publishers.count());
    for (std::size_t i = 0; i < publishers.count(); i++)
    {
        parseIdMetaTag(publishers[i], data.publishers[i]);
    }
    auto genresIds = json["genres_ids"].toArray();
    data.genresIds.reserve(genresIds.count());
    for (const QJsonValue &genreId : std::as_const(genresIds))
    {
        data.genresIds << genreId.toString();
    }
    auto genres = json["genres"].toArray();
    data.genres.resize(genres.count());
    for (std::size_t i = 0; i < genres.count(); i++)
    {
        parseLocalizedMetaTag(genres[i], data.genres[i]);
    }
    auto themesIds = json["themes_ids"].toArray();
    data.themesIds.reserve(themesIds.count());
    for (const QJsonValue &themeId : std::as_const(themesIds))
    {
        data.themesIds << themeId.toString();
    }
    auto themes = json["themes"].toArray();
    data.themes.resize(themes.count());
    for (std::size_t i = 0; i < themes.count(); i++)
    {
        parseLocalizedMetaTag(themes[i], data.themes[i]);
    }
    auto screenshots = json["screenshots"].toArray();
    data.screenshots.reserve(screenshots.count());
    for (const QJsonValue &screenshot : std::as_const(screenshots))
    {
        data.screenshots << screenshot["url_format"].toString();
    }
    auto videos = json["videos"].toArray();
    data.videos.resize(videos.count());
    for (std::size_t i = 0; i < videos.count(); i++)
    {
        parseNamedVideo(videos[i], data.videos[i]);
    }
    auto artworks = json["artworks"].toArray();
    data.artworks.reserve(artworks.count());
    for (const QJsonValue &artwork : std::as_const(artworks))
    {
        data.artworks << artwork["url_format"].toString();
    }
    data.summary = json["summary"].toObject().toVariantMap();
    data.visibleInLibrary = json["visible_in_library"].toBool();
    if (!json["aggregated_rating"].isNull())
    {
        data.aggregatedRating = json["aggregated_rating"].toDouble();
    }
    auto gameModes = json["game_modes"].toArray();
    data.gameModes.resize(gameModes.count());
    for (std::size_t i = 0; i < gameModes.count(); i++)
    {
        parseIdMetaTag(gameModes[i], data.gameModes[i]);
    }
    data.horizontalArtwork = json["horizontal_artwork"]["url_format"].toString();
    data.background = json["background"]["url_format"].toString();
    data.verticalCover = json["vertical_cover"]["url_format"].toString();
    data.cover = json["cover"]["url_format"].toString();
    data.logo = json["logo"]["url_format"].toString();
    data.squareIcon = json["square_icon"]["url_format"].toString();
    data.globalPopularityAllTime = json["global_popularity_all_time"].toInt();
    data.globalPopularityCurrent = json["global_popularity_current"].toInt();
}

void parseIdMetaTag(const QJsonValue &json, api::IdMetaTag &data)
{
    data.id = json["id"].toString();
    data.name = json["name"].toString();
    data.slug = json["slug"].toString();
}

void parseLocalizedMetaTag(const QJsonValue &json, api::LocalizedMetaTag &data)
{
    data.id = json["id"].toString();
    data.name = json["name"].toObject().toVariantMap();
    data.slug = json["slug"].toString();
}

void parseNamedVideo(const QJsonValue &json, api::NamedVideo &data)
{
    data.provider = json["provider"].toString();
    data.videoId = json["video_id"].toString();
    data.thumbnailId = json["thumbnail_id"].toString();
    data.name = json["name"].toString();
}

void parsePlatformRelease(const QJsonValue &json, api::PlatformRelease &data)
{
    data.id = json["id"].toString();
    data.platformId = json["platform_id"].toString();
    data.externalId = json["external_id"].toString();
    data.releasePerPlatformId = json["release_per_platform_id"].toString();
}
