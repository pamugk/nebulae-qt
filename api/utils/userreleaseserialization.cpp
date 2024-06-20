#include "userreleaseserialization.h"

#include <QJsonArray>

void parseUserRelease(const QJsonObject &json, api::UserRelease &data)
{
    data.platformId = json["platform_id"].toString();
    data.externalId = json["external_id"].toString();
    auto tags = json["tags"].toArray();
    for (const QJsonValue &tag : std::as_const(tags))
    {
        data.tags.append(tag["name"].toString());
    }
    if (!json["my_rating"].isNull())
    {
        data.rating = json["my_rating"].toInt();
    }
    data.dateCreated = QDateTime::fromSecsSinceEpoch(json["date_created"].toInteger());
    if (!json["owned_since"].isNull())
    {
        data.ownedSince = QDateTime::fromSecsSinceEpoch(json["owned_since"].toInteger());
    }
    if (!json["hidden"].isNull())
    {
        data.hidden = json["hidden"].toBool();
    }
    data.certificate = json["certificate"].toString();
    data.owned = json["owned"].toBool();
}

void parseGetUserReleasesResponse(const QJsonObject &json, api::GetUserReleasesResponse &data)
{
    data.totalCount = json["total_count"].toInt();
    data.limit = json["limit"].toInt();
    auto items = json["items"].toArray();
    data.items.resize(items.count());
    for (std::size_t i = 0; i < items.count(); i++)
    {
        parseUserRelease(items[i].toObject(), data.items[i]);
    }
}
