#include "./ownedproductserialization.h"

#include <QJsonArray>

void parseTag(const QJsonObject &json, api::Tag &data)
{
    data.id = json["id"].toString();
    data.name = json["name"].toString();
    data.productCount = json["productCount"].toInt();
}

void parseOwnedProduct(const QJsonObject &json, api::OwnedProduct &data)
{
    data.galaxyCompatible = json["isGalaxyCompatible"].toBool();
    auto tagIds = json["tags"].toArray();
    data.tags.resize(tagIds.count());
    for (int i = 0; i < tagIds.count(); i++)
    {
        data.tags[i] = tagIds[i].toString();
    }
    data.id = json["id"].toInteger();
    if (json["availability"].isObject())
    {
        auto availability = json["availability"];
        data.availability.available = availability["isAvailable"].toBool();
        data.availability.availableInAccount = availability["isAvailableInAccount"].toBool();
    }
    data.title = json["title"].toString();
    data.image = json["image"].toString();
    data.url = json["url"].toString();
    if (json["worksOn"].isObject())
    {
        auto worksOn = json["worksOn"];
        data.worksOn.Windows = worksOn["Windows"].toBool();
        data.worksOn.Mac = worksOn["Mac"].toBool();
        data.worksOn.Linux = worksOn["Linux"].toBool();
    }
    data.category = json["category"].toString();
    data.rating = json["rating"].toInt();
    data.comingSoon = json["isComingSoon"].toBool();
    data.movie = json["isMovie"].toBool();
    data.game = json["isGame"].toBool();
    data.slug = json["slug"].toString();
    data.updates = json["updates"].toInt();
    data.isNew = json["isNew"].toBool();
    data.dlcCount = json["dlcCount"].toInt();
    data.releaseDate.date = QDateTime::fromString(json["releaseDate"]["date"].toString(), Qt::ISODateWithMs);
    data.releaseDate.timezoneType = json["releaseDate"]["timezone_type"].toInt();
    data.releaseDate.timezone = json["releaseDate"]["timezone"].toString();
    data.baseProductMissing = json["isBaseProductMissing"].toBool();
    data.hiddingDisabled = json["isHidingDisabled"].toBool();
    data.inDevelopment = json["isBaseProductMissing"].toBool();
    data.hidden = json["isHidden"].toBool();
}

void parseOwnedProductsResponse(const QJsonObject &json, api::GetOwnedProductsResponse &data)
{
    data.page = json["page"].toInt();
    data.totalPages = json["totalPages"].toInt();
    data.totalProducts = json["totalProducts"].toInt();
    data.productsPerPage = json["productsPerPage"].toInt();
    data.moviesCount = json["moviesCount"].toInt();
    data.hasHiddenProducts = json["hasHiddenProducts"].toBool();
    data.updatedProductsCount = json["updatedProductsCount"].toInt();
    data.hiddenUpdatedProductsCount = json["hiddenUpdatedProductsCount"].toInt();

    auto tags = json["tags"].toArray();
    data.tags.resize(tags.count());
    for (int i = 0; i < tags.count(); i++)
    {
        parseTag(tags[i].toObject(), data.tags[i]);
    }

    auto products = json["products"].toArray();
    data.products.resize(products.count());
    for (int i = 0; i < products.count(); i++)
    {
        parseOwnedProduct(products[i].toObject(), data.products[i]);
    }
}
