#include "newsserialization.h"

#include <QJsonArray>

void parseNewsItem(const QJsonObject &json, api::NewsItem &data)
{
    data.id = json["id"].toInteger();
    data.title = json["title"].toString();
    data.slug = json["slug"].toString();
    data.link = json["link"].toString();
    data.body = json["body"].toString();
    data.publishDate = QDateTime::fromString(json["publish_date"].toString(), Qt::ISODate);
    data.language = json["language"].toString();
    data.commentsCount = json["comments_count"].toInt();
    data.forumThreadLink = json["forum_thread_link"].toString();

    data.imageSmall = json["images"]["small"].toString().replace(".jpg", "_news_tile.jpg");
    if (!data.imageSmall.isNull())
    {
        data.imageSmall.prepend("https:");
    }
    data.imageLarge = json["images"]["big"].toString();
    if (!data.imageLarge.isNull())
    {
        data.imageLarge.prepend("https:");
    }
}

void parseNewsResponse(const QJsonObject &json, api::GetNewsResponse &data)
{
    data.totalCount = json["total_count"].toInt();
    data.limit = json["limit"].toInt();
    data.pageToken = json["page_token"].toInt();
    data.nextPageToken = json["next_page_token"].toInt();
    auto items = json["items"].toArray();
    data.items.resize(items.count());
    for (int i = 0; i < items.count(); i++)
    {
        parseNewsItem(items[i].toObject(), data.items[i]);
    }
}
