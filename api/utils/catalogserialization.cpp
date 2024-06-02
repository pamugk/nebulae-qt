#include "./catalogserialization.h"

#include <QJsonArray>

void parseMetaTag(const QJsonObject &json, api::MetaTag &data)
{
    data.name = json["name"].toString();
    data.slug = json["slug"].toString();
}

void parseMetaTagArray(const QJsonArray &json, QVector<api::MetaTag> &data)
{
    data.resize(json.count());
    for (int i = 0; i < json.count(); i++)
    {
        parseMetaTag(json[i].toObject(), data[i]);
    }
}

void parseStringArray(const QJsonArray &json, QVector<QString> &data)
{    data.resize(json.count());
     for (int i = 0; i < json.count(); i++)
     {
         data[i] = json[i].toString();
     }
}

void parseMoney(const QJsonObject &json, api::Money &data)
{
    data.amount = json["amount"].toString();
    data.currency = json["currency"].toString();
}

void parseProduct(const QJsonObject &json, api::CatalogProduct &data)
{
    data.id = json["id"].toString();
    data.slug = json["slug"].toString();
    parseMetaTagArray(json["features"].toArray(), data.features);
    parseStringArray(json["screenshots"].toArray(), data.screenshots);
    if (json["userPreferredLanguage"].isObject())
    {
        auto preferredLanguage = json["userPreferredLanguage"];
        data.userPreferredLanguage.code = preferredLanguage["code"].toString();
        data.userPreferredLanguage.inAudio = preferredLanguage["inAudio"].toBool();
        data.userPreferredLanguage.inText = preferredLanguage["inText"].toBool();
    }
    data.releaseDate = json["releaseDate"].toString();
    data.productType = json["productType"].toString();
    data.title = json["title"].toString();
    data.coverHorizontal = json["coverHorizontal"].toString().replace(".png", "_product_tile_extended_432x243.webp");
    data.coverVertical = json["coverVertical"].toString().replace(".jpg", "_product_tile_80x114.webp");
    parseStringArray(json["developers"].toArray(), data.developers);
    parseStringArray(json["publishers"].toArray(), data.publishers);
    parseStringArray(json["operatingSystems"].toArray(), data.operatingSystems);
    if (json["price"].isObject())
    {
        auto price = json["price"];
        data.price.final = price["final"].toString();
        data.price.base = price["base"].toString();
        data.price.discount = price["discount"].toString();
        parseMoney(json["finalMoney"].toObject(), data.price.finalMoney);
        parseMoney(json["baseMoney"].toObject(), data.price.baseMoney);
    }
    data.productState = json["productState"].toString();
    parseMetaTagArray(json["genres"].toArray(), data.genres);
    parseMetaTagArray(json["tags"].toArray(), data.tags);
    data.reviewsRating = json["reviewsRating"].toInt();
}

void parseFilters(const QJsonObject &json, api::StoreFilters &data)
{
    if (json["releaseDateRange"].isObject())
    {
        auto releaseDateRange = json["releaseDateRange"];
        data.releaseDateRange.min = releaseDateRange["min"].toInt();
        data.releaseDateRange.max = releaseDateRange["max"].toInt();
    }
    if (json["priceRange"].isObject())
    {
        auto priceRange = json["priceRange"];
        data.priceRange.min = priceRange["min"].toInt();
        data.priceRange.max = priceRange["max"].toInt();
        data.priceRange.currency = priceRange["currency"].toString();
        data.priceRange.decimalPlaces = priceRange["decimalPlaces"].toInt();
    }
    parseMetaTagArray(json["genres"].toArray(), data.genres);
    parseMetaTagArray(json["languages"].toArray(), data.languages);
    parseMetaTagArray(json["systems"].toArray(), data.systems);
    parseMetaTagArray(json["tags"].toArray(), data.tags);
    data.discounted = json["discounted"].toBool();
    parseMetaTagArray(json["features"].toArray(), data.features);
    parseMetaTagArray(json["releaseStatuses"].toArray(), data.releaseStatuses);
    parseStringArray(json["types"].toArray(), data.types);
    auto fullGenresList = json["fullGenresList"].toArray();
    data.fullGenresList.resize(fullGenresList.count());
    for (int i = 0; i < fullGenresList.count(); i++)
    {
        auto genre = fullGenresList[i].toObject();
        data.fullGenresList[i].name = genre["name"].toString();
        data.fullGenresList[i].slug = genre["slug"].toString();
        data.fullGenresList[i].level = genre["level"].toInt();
    }
    parseMetaTagArray(json["fullTagsList"].toArray(), data.fullTagsList);
}

void parseSearchCatalogResponse(const QJsonObject &json, api::SearchCatalogResponse &data)
{
    data.pages = json["pages"].toInt();
    data.productCount = json["productCount"].toInt();
    auto products = json["products"].toArray();
    data.products.resize(products.count());
    for (int i = 0; i < products.count(); i++)
    {
        parseProduct(products[i].toObject(), data.products[i]);
    }
    parseFilters(json["filters"].toObject(), data.filters);
}
