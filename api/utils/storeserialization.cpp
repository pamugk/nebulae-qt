#include "storeserialization.h"

#include <QJsonArray>
#include <QMap>

#include "catalogserialization.h"

void parsePrice(const QJsonObject &json, api::StoreProductPrice &data)
{
    data.baseAmount = json["baseAmount"].toString().toDouble();
    data.finalAmount = json["finalAmount"].toString().toDouble();
    data.discountPercentage = json["discountPercentage"].toInt();
    data.free = json["isFree"].toBool();
}

void parseProduct(const QJsonObject &json, api::StoreProduct &data, const QString &imageFormatter)
{
    data.id = json["id"].toString().toULongLong();
    data.title = json["title"].toString();
    data.image = json["image"].toString();
    if (!data.image.isNull())
    {
        data.image.prepend("https:").append(imageFormatter);
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

void parseCustomSectionItem(const QJsonObject &json, api::StoreCustomSectionItem &data,
                            const QString &coverFormat)
{
    data.dealActiveFrom = QDateTime::fromString(json["dealActiveFrom"].toString(), Qt::ISODate);
    data.dealActiveTo = QDateTime::fromString(json["dealActiveTo"].toString(), Qt::ISODate);
    parseProduct(json["product"].toObject(), data.product, coverFormat);
}

void parseGetStoreCustomSectionResponse(const QJsonObject &json, api::GetStoreCustomSectionResponse &data,
                                        const QString &coverFormat)
{
    data.id = json["id"].toString();

    auto items = json["products"]["items"].toArray();
    data.items.resize(items.count());
    for (int i = 0; i < items.count(); i++)
    {
        parseCustomSectionItem(items[i].toObject(), data.items[i], coverFormat);
    }

    data.visibleFrom = QDateTime::fromString(json["products"]["currentServerTime"].toString(), Qt::ISODate);
    data.visibleFrom = QDateTime::fromString(json["visibleFrom"].toString(), Qt::ISODate);
    data.visibleTo = QDateTime::fromString(json["visibleTo"].toString(), Qt::ISODate);
}

void parseGetStoreDiscoverGamesResponse(const QJsonObject &json, api::GetStoreDiscoverGamesSectionResponse &data)
{
    auto personalizedProducts = json["personalizedProducts"].toArray();
    data.personalizedProducts.resize(personalizedProducts.count());
    for (std::size_t i = 0; i < personalizedProducts.count(); i++)
    {
        parseProduct(personalizedProducts[i].toObject(), data.personalizedProducts[i], "_product_tile_136.webp");
    }
}

void parseStoreNowOnSaleTab(const QJsonObject &json, api::StoreNowOnSaleTab &data)
{
    data.id = json["id"].toString();
    data.title = json["title"].toString();

    auto bigThingy = json["bigThingy"];
    auto colorRgbArray = bigThingy["color_as_rgb_array"].toArray();
    data.bigThingy.background = bigThingy["background"].toString().prepend("https:");
    data.bigThingy.color = bigThingy["color"].toString();
    data.bigThingy.colorRgbArray =
    {
        static_cast<unsigned char>(colorRgbArray[0].toInt()),
        static_cast<unsigned char>(colorRgbArray[1].toInt()),
        static_cast<unsigned char>(colorRgbArray[2].toInt()),
    };
    data.bigThingy.text = bigThingy["text"].toString();
    data.bigThingy.textSlug = bigThingy["textSlug"].toString();
    data.bigThingy.discountValue = bigThingy["discountValue"].toInt();
    data.bigThingy.discountUpTo = bigThingy["discountUpTo"].toBool();
    data.bigThingy.url = bigThingy["url"].toString();
    data.bigThingy.countdownDate = QDateTime::fromMSecsSinceEpoch(bigThingy["countdownDate"].toInteger());
}

void parseGetStoreNowOnSaleResponse(const QJsonObject &json, api::GetStoreNowOnSaleResponse &data)
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
            parseCatalogProduct(product.toObject(), data.products[productIdx], "_product_tile_256.webp");
        }
    }

    auto tabs = json["tabs"].toArray();
    data.tabs.resize(tabs.count());
    for (std::size_t i = 0; i < tabs.count(); i++)
    {
        parseStoreNowOnSaleTab(tabs[i].toObject(), data.tabs[i]);
    }
}
