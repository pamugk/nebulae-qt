#include "storeserialization.h"

#include <QJsonArray>
#include <QMap>

#include "catalogserialization.h"

void parsePrice(const QJsonValue &json, api::StoreProductPrice &data)
{
    data.baseAmount = json["baseAmount"].toString().toDouble();
    data.finalAmount = json["finalAmount"].toString().toDouble();
    data.discountPercentage = json["discountPercentage"].toInt();
    data.free = json["isFree"].toBool();
}

void parseProduct(const QJsonValue &json, api::StoreProduct &data, const QString &imageFormatter)
{
    data.id = json["id"].toString();
    data.title = json["title"].toString();
    data.image = json["image"].toString();
    if (!data.image.isNull())
    {
        data.image.prepend("https:").append(imageFormatter);
    }
    parsePrice(json["price"], data.price);

    auto operatingSystems = json["supportedOperatingSystems"].toArray();
    data.supportedOperatingSystems.reserve(operatingSystems.count());
    for (const QJsonValue &os : std::as_const(operatingSystems))
    {
        data.supportedOperatingSystems << os.toString();
    }

    data.comingSoon = json["isComingSoon"].toBool();
    data.inDevelopment = json["inDevelopment"].toBool();
    data.buyable = json["buyable"].toBool();
    data.availableForSale = json["isAvailableForSale"].toBool();
    data.movie = json["isMovie"].toBool();
    data.visibleInCatalog = json["isVisibleInCatalog"].toBool();
    data.preorder = json["isPreorder"].toBool();
}

void parseCustomSectionItem(const QJsonValue &json, api::StoreCustomSectionItem &data,
                            const QString &coverFormat)
{
    data.dealActiveFrom = QDateTime::fromString(json["dealActiveFrom"].toString(), Qt::ISODate);
    data.dealActiveTo = QDateTime::fromString(json["dealActiveTo"].toString(), Qt::ISODate);
    parseProduct(json["product"], data.product, coverFormat);
}

void parseGetStoreCustomSectionResponse(const QJsonValue &json, api::GetStoreCustomSectionResponse &data,
                                        const QString &coverFormat)
{
    data.id = json["id"].toString();

    auto items = json["products"]["items"].toArray();
    data.items.resize(items.count());
    for (std::size_t i = 0; i < items.count(); i++)
    {
        parseCustomSectionItem(items[i], data.items[i], coverFormat);
    }

    data.visibleFrom = QDateTime::fromString(json["products"]["currentServerTime"].toString(), Qt::ISODate);
    data.visibleFrom = QDateTime::fromString(json["visibleFrom"].toString(), Qt::ISODate);
    data.visibleTo = QDateTime::fromString(json["visibleTo"].toString(), Qt::ISODate);
}

void parseGetStoreDiscoverGamesResponse(const QJsonValue &json, api::GetStoreDiscoverGamesSectionResponse &data)
{
    auto personalizedProducts = json["personalizedProducts"].toArray();
    data.personalizedProducts.resize(personalizedProducts.count());
    for (std::size_t i = 0; i < personalizedProducts.count(); i++)
    {
        parseProduct(personalizedProducts[i], data.personalizedProducts[i], "_product_tile_136.webp");
    }
}

void parseStoreNowOnSaleTabCard(const QJsonValue &json, api::StoreNowOnSaleTabCard &data)
{
    auto colorRgbArray = json["color_as_rgb_array"].toArray();
    data.background = json["background"].toString().prepend("https:");
    data.color = json["color"].toString();
    data.colorRgbArray =
    {
        static_cast<unsigned char>(colorRgbArray[0].toInt()),
        static_cast<unsigned char>(colorRgbArray[1].toInt()),
        static_cast<unsigned char>(colorRgbArray[2].toInt()),
    };
    data.text = json["text"].toString();
    data.textSlug = json["textSlug"].toString();
    data.discountValue = json["discountValue"].toInt();
    data.discountUpTo = json["discountUpTo"].toBool();
    data.url = json["url"].toString();
    data.countdownDate = QDateTime::fromMSecsSinceEpoch(json["countdownDate"].toInteger());
}

void parseStoreNowOnSaleTab(const QJsonValue &json, api::StoreNowOnSaleTab &data)
{
    data.id = json["id"].toString();
    data.title = json["title"].toString();
    parseStoreNowOnSaleTabCard(json["bigThingy"], data.bigThingy);
}

void parseGetStoreNowOnSaleResponse(const QJsonValue &json, api::GetStoreNowOnSaleResponse &data)
{
    auto productIds = json["allDiscounts"]["products"].toArray();
    QMap<QString, std::size_t> productOrder;
    for (std::size_t i = 0; i < productIds.count(); i++)
    {
        QString productId = productIds[i].toString();
        productOrder[productId] = i;
    }

    auto products = json["allProducts"].toArray();
    data.products.resize(productIds.count());
    for (const QJsonValue &product : std::as_const(products))
    {
        QString productId = product["id"].toString();
        if (productOrder.contains(productId))
        {
            auto productIdx = productOrder[productId];
            parseCatalogProduct(product, data.products[productIdx], "_product_tile_256.webp");
        }
    }

    auto tabs = json["tabs"].toArray();
    data.tabs.resize(tabs.count());
    for (std::size_t i = 0; i < tabs.count(); i++)
    {
        parseStoreNowOnSaleTab(tabs[i], data.tabs[i]);
    }
}

void parseGetStoreNowOnSaleSectionResponse(const QJsonValue &json, api::GetStoreNowOnSaleSectionResponse &data)
{
    data.id = json["id"].toString();
    auto personalizedProducts = json["personalizedProducts"].toArray();
    data.personalizedProducts.resize(personalizedProducts.count());
    for (std::size_t i = 0; i < personalizedProducts.count(); i++)
    {
        parseProduct(personalizedProducts[i], data.personalizedProducts[i], "_product_tile_256.webp");
    }
    parseStoreNowOnSaleTabCard(json["bigThingy"], data.bigThingy);
}

void parseGetStoreRecommendedDlcsResponse(const QJsonValue &json, api::GetStoreRecommendedDlcsResponse &data)
{
    data.hasRecommendations = json["hasRecommendations"].toBool();
    auto recommendations = json["recommendations"].toArray();
    data.recommendations.resize(recommendations.count());
    for (std::size_t i = 0; i < recommendations.count(); i++)
    {
        parseProduct(recommendations[i], data.recommendations[i], "_product_tile_256.webp");
    }
}
