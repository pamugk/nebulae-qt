#include "recommendationserialization.h"

#include <QJsonArray>

void parseRecommendation(const QJsonObject &json, api::Recommendation &data)
{
    data.productId = json["product_id"].toInteger();
    data.rating = json["rating"].toDouble();

    auto details = json["details"];
    data.details.title = details["title"].toString();
    data.details.available = details["is_available"].toBool();
    data.details.imageUrl = details["image_url"].toString();
    data.details.imageHorizontalUrl = details["image_horizontal_url"].toString();
    data.details.storeUrl = details["store_url"].toString();
    data.details.feedback = details["feedback"].toDouble();

    auto pricing = json["pricing"];
    data.pricing.currency = pricing["currency"].toString();
    data.pricing.country = pricing["country"].toString();
    data.pricing.priceSet = pricing["price"].isObject();
    data.pricing.basePrice = pricing["price"]["base_price"].toInt();
    data.pricing.finalPrice = pricing["price"]["final_price"].toInt();
    data.pricing.discount.id = pricing["discount"]["id"].toString();
    data.pricing.discount.basisPoints = pricing["discount"]["basis_points"].toInt();

}

void parseCatalogProductInfoResponse(const QJsonObject &json, api::GetRecommendationsResponse &data)
{
    data.context = json["context"].toString();
    data.requestId = json["request_id"].toString();
    auto products = json["products"].toArray();
    data.products.resize(products.count());
    for (int i = 0; i < products.count(); i++)
    {
        parseRecommendation(products[i].toObject(), data.products[i]);
    }
}
