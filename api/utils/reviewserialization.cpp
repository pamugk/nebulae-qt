#include "./reviewserialization.h"

#include <QJsonArray>

void parseRatingResponse(const QJsonValue &json, api::GetRatingResponse &data)
{
    data.value = json["value"].toDouble();
    data.count = json["count"].toInt();
}

void parseReview(const QJsonValue &json, api::Review &data)
{
    data.id = json["id"].toString();
    data.productId = json["productId"].toString();
    data.rating = json["rating"]["value"].toDouble();
    data.contentTitle = json["content"]["title"].toString();
    data.contentDescription = json["content"]["description"].toString();
    data.contentLanguage = json["content"]["language"].toString();
    data.downvotes = json["votes"]["downvotes"].toInt();
    data.upvotes = json["votes"]["upvotes"].toInt();
    data.date = QDateTime::fromString(json["date"].toString(),Qt::ISODate);
    data.creationDate = QDateTime::fromString(json["creationDate"].toString(),Qt::ISODate);

    auto labels = json["labels"].toArray();
    data.labels.reserve(labels.count());
    for (const QJsonValue &label : std::as_const(labels))
    {
        data.labels << label.toString();
    }

    data.reviewer.id = json["reviewer"]["id"].toString();
    data.reviewer.username = json["reviewer"]["username"].toString();
    data.reviewer.counters.games = json["reviewer"]["counters"]["games"].toInt();
    data.reviewer.counters.reviews = json["reviewer"]["counters"]["reviews"].toInt();
    auto avatar = json["reviewer"]["avatar"]["links"].toVariant().toMap();
    for (const auto &[avatarLink, value] : avatar.asKeyValueRange())
    {
        data.reviewer.avatar[avatarLink] = value.toString();
    }
}

void parseReviewsResponse(const QJsonValue &json, api::GetReviewsResponse &data)
{
    data.page = json["page"].toInt();
    data.pages = json["pages"].toInt();
    data.limit = json["limit"].toInt();
    data.reviewCount = json["reviewCount"].toInt();
    data.overallAvgRating = json["overallAvgRating"].toDouble();
    data.filteredAvgRating = json["filteredAvgRating"].toDouble();
    data.reviewable = json["isReviewable"].toBool();
    data.mostHelpfulReviewId = json["mostHelpful"]["id"].toString();

    auto items = json["_embedded"]["items"].toArray();
    data.items.resize(items.count());
    for (std::size_t i = 0; i < items.count(); i++)
    {
        parseReview(items[i], data.items[i]);
    }
}
