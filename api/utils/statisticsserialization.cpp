#include "./statisticsserialization.h"

#include <QJsonArray>

#include "platformachievementserialization.h"

void parsePlatformUserAchievement(const QJsonValue &json, api::PlatformUserAchievement &data)
{
    parsePlatformAchievement(json, data);
    if (!json["date_unlocked"].isNull())
    {
        data.dateUnlocked = QDateTime::fromSecsSinceEpoch(json["date_unlocked"].toInteger());
    }
}

void parseGetUserPlatformAchievementsResponse(const QJsonValue &json, api::GetUserPlatformAchievementsResponse &data)
{
    data.limit = json["limit"].toInt();
    data.pageToken = json["page_token"].toString();
    data.nextPageToken = json["next_page_token"].toString();
    auto items = json["items"].toArray();
    data.items.resize(items.count());
    for (std::size_t i = 0; i < items.count(); i++)
    {
        parsePlatformUserAchievement(items[i], data.items[i]);
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
