#include "storeserialization.h"

#include <QJsonArray>

void parsePrice(const QJsonObject &json, api::StoreDiscoveredProductPrice &data)
{
    data.baseAmount = json["baseAmount"].toString().toDouble();
    data.finalAmount = json["finalAmount"].toString().toDouble();
    data.discountPercentage = json["discountPercentage"].toInt();
    data.free = json["isFree"].toBool();
}

void parseProduct(const QJsonObject &json, api::StoreDiscoveredProduct &data)
{
    data.id = json["id"].toString().toULongLong();
    data.title = json["title"].toString();
    data.image = json["image"].toString();
    if (!data.image.isNull())
    {
        data.image.prepend("https:").append("_product_tile_136.webp");
    }
    parsePrice(json["price"].toObject(), data.price);

    auto operatingSystems = json["supportedOperatingSystems"].toArray();
    data.supportedOperatingSystems.resize(operatingSystems.count());
    for (const QJsonValue &os : std::as_const(operatingSystems))
    {
        data.supportedOperatingSystems.append(os.toString());
    }

    data.comingSoon = json["isComingSoon"].toBool();
    data.inDevelopment = json["inDevelopment"].toBool();
    data.buyable = json["buyable"].toBool();
    data.availableForSale = json["isAvailableForSale"].toBool();
    data.movie = json["isMovie"].toBool();
    data.visibleInCatalog = json["isVisibleInCatalog"].toBool();
    data.preorder = json["isPreorder"].toBool();
}

void parseGetStoreDiscoverGamesResponse(const QJsonObject &json, api::GetStoreDiscoverGamesSectionResponse &data)
{
    auto personalizedProducts = json["personalizedProducts"].toArray();
    data.personalizedProducts.resize(personalizedProducts.count());
    for (std::size_t i = 0; i < personalizedProducts.count(); i++)
    {
        parseProduct(personalizedProducts[i].toObject(), data.personalizedProducts[i]);
    }
}
