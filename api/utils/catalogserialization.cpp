#include "./catalogserialization.h"

#include <QJsonArray>

void parseMetaTag(const QJsonValue &json, api::MetaTag &data)
{
    data.name = json["name"].toString();
    data.slug = json["slug"].toString();
}

void parseMetaTagArray(const QJsonArray &json, QVector<api::MetaTag> &data)
{
    data.resize(json.count());
    for (std::size_t i = 0; i < json.count(); i++)
    {
        parseMetaTag(json[i], data[i]);
    }
}

void parseStringArray(const QJsonArray &json, QVector<QString> &data)
{
    data.resize(json.count());
    for (std::size_t i = 0; i < json.count(); i++)
    {
        data[i] = json[i].toString();
    }
}

void parseMoney(const QJsonValue &json, api::Money &data)
{
    data.amount = json["amount"].toString().toDouble();
    data.currency = json["currency"].toString();
}

void parseCatalogProduct(const QJsonValue &json, api::CatalogProduct &data, const QString &horizontalCoverFormat)
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
    else
    {
        data.userPreferredLanguage = {};
    }
    data.releaseDate = json["releaseDate"].toString();
    data.productType = json["productType"].toString();
    data.title = json["title"].toString();
    data.coverHorizontal = json["coverHorizontal"].toString().replace(".png", horizontalCoverFormat);
    data.coverVertical = json["coverVertical"].toString().replace(".jpg", "_product_tile_80x114.webp");
    parseStringArray(json["developers"].toArray(), data.developers);
    parseStringArray(json["publishers"].toArray(), data.publishers);
    parseStringArray(json["operatingSystems"].toArray(), data.operatingSystems);
    if (json["price"].isObject())
    {
        auto priceJson = json["price"];
        api::CatalogPrice price = { priceJson["final"].toString(), priceJson["base"].toString(), priceJson["discount"].toString(), {}, {} };
        parseMoney(priceJson["finalMoney"], price.finalMoney);
        parseMoney(priceJson["baseMoney"], price.baseMoney);
        data.price = price;
    }
    else
    {
        data.price = std::nullopt;
    }
    data.productState = json["productState"].toString();
    parseMetaTagArray(json["genres"].toArray(), data.genres);
    parseMetaTagArray(json["tags"].toArray(), data.tags);
    data.reviewsRating = json["reviewsRating"].toInt();
}

void parseFilters(const QJsonValue &json, api::StoreFilters &data)
{
    if (json["releaseDateRange"].isObject())
    {
        auto releaseDateRange = json["releaseDateRange"];
        data.releaseDateRange.min = releaseDateRange["min"].toInt();
        data.releaseDateRange.max = releaseDateRange["max"].toInt();
    }
    else
    {
        data.releaseDateRange = {};
    }
    if (json["priceRange"].isObject())
    {
        auto priceRange = json["priceRange"];
        data.priceRange.min = priceRange["min"].toInt();
        data.priceRange.max = priceRange["max"].toInt();
        data.priceRange.currency = priceRange["currency"].toString();
        data.priceRange.decimalPlaces = priceRange["decimalPlaces"].toInt();
    }
    else
    {
        data.priceRange = {};
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
    for (std::size_t i = 0; i < fullGenresList.count(); i++)
    {
        const QJsonValue &genre = fullGenresList[i];
        data.fullGenresList[i].name = genre["name"].toString();
        data.fullGenresList[i].slug = genre["slug"].toString();
        data.fullGenresList[i].level = genre["level"].toInt();
    }
    parseMetaTagArray(json["fullTagsList"].toArray(), data.fullTagsList);
}

void parseSearchCatalogResponse(const QJsonValue &json, api::SearchCatalogResponse &data)
{
    data.pages = json["pages"].toInt();
    data.productCount = json["productCount"].toInt();
    auto products = json["products"].toArray();
    data.products.resize(products.count());
    for (std::size_t i = 0; i < products.count(); i++)
    {
        parseCatalogProduct(products[i], data.products[i], "_product_tile_extended_432x243.webp");
    }
    parseFilters(json["filters"], data.filters);
}
