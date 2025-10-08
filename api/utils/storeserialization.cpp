#include "storeserialization.h"

#include <QJsonArray>
#include <QMap>

#include "catalogserialization.h"

void parseGetStoreProductsSectionResponse(const QJsonValue &json, api::GetStoreProductsSectionResponse &data)
{
    const auto properties = json["properties"];
    data.sectionId = properties["sectionId"].toString();
    data.contentSourceType = properties["contentSourceType"].toString();
    data.title = properties["title"].toString();
    data.description = properties["description"].toString();
    auto items = properties["items"].toArray();
    data.items.resize(items.count());
    for (std::size_t i = 0; i < items.count(); i++)
    {
        parseCatalogProduct(items[i], data.items[i], "_product_tile_256.webp");
    }
    data.seeMoreLink = properties["seeMoreLink"].toString();
}

void parseSection(const QJsonValue &json, api::StoreSection &data)
{
    data.id = json["sectionId"].toString();
    data.sectionType = json["sectionType"].toString();
    data.personalized = json["isPersonalized"].toBool();
    data.hideOnLoad = json["hideOnLoad"].toBool();
    data.loadOnEmbed = json["loadOnEmbed"].toBool();
    data.contentSourceType = json["contentSourceType"].toString();
}

void parseGetStoreSectionsResponse(const QJsonValue &json, api::GetStoreSectionsResponse &data)
{
    auto sections = json["sections"].toArray();
    data.sections.resize(sections.count());
    for (std::size_t i = 0; i < sections.count(); i++)
    {
        parseSection(sections[i], data.sections[i]);
    }
}
