#include "./statisticsserialization.h"

#include <QJsonArray>

void parseAchievement(const QJsonObject &json, api::Achievement &data)
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

void parseGetAchievementsResponse(const QJsonObject &json, api::GetAchievementsResponse &data)
{
    data.totalCount = json["total_count"].toInt();
    data.limit = json["limit"].toInt();
    data.pageToken = json["page_token"].toString("0").toInt();
    data.hasNextPage = json["next_page_token"].isString();
    data.achievementsMode = json["achievements_mode"].toString();
    auto items = json["items"].toArray();
    data.items.resize(items.count());
    for (int i = 0; i < items.count(); i++)
    {
        parseAchievement(items[i].toObject(), data.items[i]);
    }
}

void parseGetSessionsResponse(const QJsonObject &json, api::GetPlaySessionsResponse &data)
{
    data.totalSum = json["total_sum"].toInt();
    auto gameTime = json["game_time"].toArray();
    foreach (const QJsonValue &item, gameTime)
    {
        data.gameTime[item["game_id"].toInteger()] = item["time_sum"].toInt();
    }
}
