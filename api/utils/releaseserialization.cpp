#include "releaseserialization.h"

#include <QJsonArray>
#include <QJsonObject>

#include "gameserialization.h"

void parseRelease(const QJsonValue &json, api::Release &data)
{
    data.id = json["id"].toString();
    data.gameId = json["game_id"].toString();
    data.platformId = json["platform_id"].toString();
    data.externalId = json["external_id"].toString();
    auto dlcsIds = json["dlcs_ids"].toArray();
    data.dlcsIds.resize(dlcsIds.count());
    for (const QJsonValue &dlcId : std::as_const(dlcsIds))
    {
        data.dlcsIds.append(dlcId.toString());
    }
    auto dlcs = json["dlcs"].toArray();
    data.dlcs.resize(dlcs.count());
    for (std::size_t i = 0; i < dlcs.count(); i++)
    {
        parsePlatformRelease(dlcs[i], data.dlcs[i]);
    }
    data.parentId = json["parent_id"].toString();
    auto supportedOperatingSystems = json["supported_operating_systems"].toArray();
    data.supportedOperatingSystems.resize(supportedOperatingSystems.count());
    for (std::size_t i = 0; i < supportedOperatingSystems.count(); i++)
    {
        const QJsonValue &os = supportedOperatingSystems[i];
        data.supportedOperatingSystems[i].name = os["name"].toString();
        data.supportedOperatingSystems[i].slug = os["slug"].toString();
    }
    auto availableLanguages = json["available_languages"].toArray();
    data.availableLanguages.resize(availableLanguages.count());
    for (const QJsonValue &language : std::as_const(availableLanguages))
    {
        data.availableLanguages.append(language["code"].toString());
    }
    data.firstReleaseDate = QDateTime::fromString(json["first_release_date"].toString(), Qt::ISODate);
    parseGame(json["game"], data.game);
    data.title = json["title"].toObject().toVariantMap();
    data.sortingTitle = json["sorting_title"].toObject().toVariantMap();
    data.type = json["type"].toString();
    auto videos = json["videos"].toArray();
    data.videos.resize(videos.count());
    for (std::size_t i = 0; i < videos.count(); i++)
    {
        parseNamedVideo(videos[i], data.videos[i]);
    }
    data.summary = json["summary"].toObject().toVariantMap();
    auto gameModes = json["game_modes"].toArray();
    data.gameModes.resize(gameModes.count());
    for (std::size_t i = 0; i < videos.count(); i++)
    {
        parseIdMetaTag(gameModes[i], data.gameModes[i]);
    }
    data.logo = json["logo"]["url_format"].toString();
}
