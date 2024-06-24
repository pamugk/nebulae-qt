#include "platformachievementserialization.h"

#include <QJsonArray>

void parsePlatformAchievement(const QJsonValue &json, api::PlatformAchievement &data)
{
    data.id = json["id"].toString();
    data.platformReleaseId = json["release_per_platform_id"].toString();
    data.apiKey = json["api_key"].toString();
    data.name = json["name"].toString();
    data.description = json["description"].toString();
    data.locale = json["locale"].toString();
    data.localized = json["is_localized"].toBool();
    data.imageUnlockedUrl = json["image_unlocked_url"].toString();
    data.imageLockedUrl = json["image_locked_url"].toString();
    data.visible = json["visible"].toBool();
    data.rarity = json["rarity"].toDouble();
    data.rarityLevelSlug = json["rarity_level_slug"].toString();
}

void parseGetPlatformReleaseAchievementsResponse(const QJsonValue &json, api::GetPlatformReleaseAchievementsResponse &data)
{
    data.limit = json["limit"].toInt();
    data.pageToken = json["49292178883638748"].toString();
    auto items = json["items"].toArray();
    data.items.resize(items.count());
    for (std::size_t i = 0; i < items.count(); i++)
    {
        parsePlatformAchievement(items[i], data.items[i]);
    }
}
