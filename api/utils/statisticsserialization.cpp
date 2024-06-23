#include "./statisticsserialization.h"

#include <QJsonArray>

void parseAchievement(const QJsonValue &json, api::UserAchievement &data)
{
    data.achievementId = json["achievement_id"].toString();
    data.achievementKey = json["achievement_key"].toString();
    data.clientId = json["client_id"].toString();
    data.name = json["name"].toString();
    data.description = json["description"].toString();
    data.imageUrlUnlocked = json["image_url_unlocked"].toString();
    data.imageUrlLocked = json["image_url_locked"].toString();
    if (json["date_unlocked"].isString())
    {
        data.dateUnlocked = QDateTime::fromString(json["date_unlocked"].toString(), Qt::ISODate);
    }
    data.rarity = json["rarity"].toDouble();
    data.rarityLevelDescription = json["rarity_level_description"].toString();
    data.rarityLevelSlug = json["rarity_level_slug"].toString();
}

void parseGetAchievementsResponse(const QJsonValue &json, api::GetUserAchievementsResponse &data)
{
    data.totalCount = json["total_count"].toInt();
    data.limit = json["limit"].toInt();
    data.pageToken = json["page_token"].toString("0").toInt();
    data.hasNextPage = json["next_page_token"].isString();
    data.achievementsMode = json["achievements_mode"].toString();
    auto items = json["items"].toArray();
    data.items.resize(items.count());
    for (std::size_t i = 0; i < items.count(); i++)
    {
        parseAchievement(items[i], data.items[i]);
    }
}
void parseGetUserGameTimeStatisticsResponse(const QJsonValue &json, api::GetUserGameTimeStatisticsResponse &data)
{
    data.limit = json["limit"].toInt();
    data.pageToken = json["page_token"].toString();
    auto items = json["items"].toArray();
    data.items.resize(items.count());
    for (std::size_t i = 0; i < items.count(); i++)
    {
        const QJsonValue &itemJson = items[i];
        auto &item = data.items[i];
        item.releasePerPlatformId = itemJson["release_per_platform_id"].toString();
        item.timeSum = itemJson["game_time"]["time_sum"].toInt();
        item.lastSessionDate = QDateTime::fromSecsSinceEpoch(itemJson["game_time"]["last_session_date"].toInteger());
    }
}
