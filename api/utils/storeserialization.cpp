#include "storeserialization.h"

#include <QJsonArray>
#include <QMap>

#include "catalogserialization.h"

void parseBannerItem(const QJsonValue &json, api::StoreBannerItem &data)
{
    data.id = json["id"].toString();
    if (json["product"].isObject() && !json["product"].isNull())
    {
        api::CatalogProduct product;
        parseCatalogProduct(json["product"], product, "_product_tile_256.webp");
        data.product = product;
    }
    data.background = json["background"]["desktop"].toString();
    data.logo = json["logo"]["desktop"].toString();
    data.gradientBaseColor = json["gradientBaseColor"]["desktop"].toString();
    data.title = json["text"]["title"].toString();
    data.subtitle = json["text"]["subtitle"].toString();
    if (json["visibleFrom"].isString())
    {
        data.visibleFrom = QDateTime::fromString(json["visibleFrom"].toString(), Qt::DateFormat::ISODateWithMs);
    }
    if (json["visibleTo"].isString())
    {
        data.visibleTo = QDateTime::fromString(json["visibleTo"].toString(), Qt::DateFormat::ISODateWithMs);
    }
    data.customProperties.url = json["customProperties"]["url"].toString();
    data.customProperties.buttonText = json["customProperties"]["buttonText"].toString();
    data.customProperties.discountText = json["customProperties"]["discountText"].toString();
    data.useDarkColorFont = json["useDarkColorFont"].toBool();
}

void parseDiscoverColumn(const QJsonValue &json, api::StoreDiscoverColumn &data)
{
    const auto items = json["items"].toArray();
    data.items.resize(items.count());
    for (std::size_t i = 0; i < items.count(); i++)
    {
        parseCatalogProduct(items[i], data.items[i], "_product_tile_256.webp");
    }
    data.title = json["title"].toString();
    data.seeMoreLink = json["seeMoreLink"].toString();
}

void parseStoreNewsItem(const QJsonValue &json, api::NewsItem &data)
{
    data.id = json["id"].toInteger();
    data.title = json["title"].toString();
    data.slug = json["slug"].toString();
    data.link = json["links"]["website"].toString();
    data.body = json["body"].toString();
    data.publishDate = QDateTime::fromString(json["publishDate"].toString(), Qt::ISODate);
    data.language = json["language"].toString();
    data.commentsCount = json["commentsCount"].toInt();
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

void parseVerticalBannerItem(const QJsonValue &json, api::StoreVerticalBannerItem &data)
{
    data.promoId = json["promoId"].toString();
    data.title = json["title"].toString();
    data.url = json["url"].toString();
    data.backgroundImage = json["backgroundImage"].toString();
    data.color = json["color"].toString();
    data.discount = json["discount"].toString();
    data.discountUpTo = json["discountIsUpTo"].toBool();
    data.promoEndDate = QDateTime::fromMSecsSinceEpoch(json["promoEndDate"].toInteger());
}

void parseGetStoreAnnouncementSectionResponse(const QJsonValue &json, api::GetStoreAnnouncementSectionResponse &data)
{
    const auto properties = json["properties"];
    data.data.id = properties["id"].toString();
    if (properties["product"].isObject() && !properties["product"].isNull())
    {
        api::CatalogProduct product;
        parseCatalogProduct(properties["product"], product, "_product_tile_256.webp");
        data.data.product = product;
    }
    data.data.background = properties["background"]["desktop"].toString();
    data.data.logo = properties["logo"]["desktop"].toString();
    data.data.gradientBaseColor = properties["gradientBaseColor"]["desktop"].toString();
    data.data.title = properties["info"].toString();
    data.data.subtitle = properties["headline"].toString();
    if (properties["visibleFrom"].isString())
    {
        data.data.visibleFrom = QDateTime::fromString(properties["visibleFrom"].toString(), Qt::DateFormat::ISODateWithMs);
    }
    if (properties["visibleTo"].isString())
    {
        data.data.visibleTo = QDateTime::fromString(properties["visibleTo"].toString(), Qt::DateFormat::ISODateWithMs);
    }
    data.data.customProperties.url = properties["url"].toString();
    data.data.customProperties.buttonText = properties["buttonText"].toString();
    data.data.customProperties.discountText = properties["discountText"].toString();
    data.data.useDarkColorFont = properties["useDarkColorFont"].toBool();
}

void parseGetStoreDiscoverSectionResponse(const QJsonValue &json, api::GetStoreDiscoverGamesSectionResponse &data)
{
    const auto properties = json["properties"];
    parseDiscoverColumn(properties["columnLeft"], data.columnLeft);
    parseDiscoverColumn(properties["columnRight"], data.columnRight);
}

void parseGetStoreHeroSectionResponse(const QJsonValue &json, api::GetStoreHeroSectionResponse &data)
{
    const auto properties = json["properties"];
    data.theme = properties["theme"].toString();
    data.title = properties["title"].toString();
    data.button.link = properties["button"]["buttonLink"].toString();
    data.button.text = properties["button"]["buttonText"].toString();
    data.button.anchor = properties["button"]["buttonAnchor"].toString();
    data.imageHash = properties["images"]["desktopImageHash"].toString();
    if (properties["endDate"].isString())
    {
        data.endDate = QDateTime::fromString(properties["endDate"].toString(), Qt::DateFormat::ISODateWithMs);
    }
    data.videoId = properties["videoId"].toString();
    data.subtitle = properties["subtitle"].toString();
    data.sectionId = properties["sectionId"].toString();
    data.description = properties["description"].toString();
    data.showCountdown = properties["showCountdown"].toBool();
    data.buttonSecondary.link = properties["buttonSecondary"]["buttonLink"].toString();
    data.buttonSecondary.text = properties["buttonSecondary"]["buttonText"].toString();
    data.buttonSecondary.anchor = properties["buttonSecondary"]["buttonAnchor"].toString();
    data.showTopModLabel = properties["showTopModLabel"].toBool();
}

void parseGetStoreHighlightsSectionResponse(const QJsonValue &json, api::GetStoreHighlightsSectionResponse &data)
{
    const auto items = json["properties"].toArray();
    data.items.resize(items.count());
    for (std::size_t i = 0; i < items.count(); i++)
    {
        parseBannerItem(items[i], data.items[i]);
    }
}

void parseGetStoreNewsSectionResponse(const QJsonValue &json, api::GetStoreNewsSectionResponse &data)
{
    auto items = json["properties"]["items"].toArray();
    data.items.resize(items.count());
    for (std::size_t i = 0; i < items.count(); i++)
    {
        parseStoreNewsItem(items[i], data.items[i]);
    }
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

void parseGetStorePromoBannerSectionResponse(const QJsonValue &json, api::GetStorePromoBannerSectionResponse &data)
{
    const auto properties = json["properties"];
    data.sectionId = properties["sectionId"].toString();
    data.hideOnLoad = properties["hideOnLoad"].toBool();
    data.buttonText = properties["buttonText"].toString();
    data.link = properties["link"].toString();
    data.image = properties["images"]["desktop"]["link"].toString();
    data.logo = properties["images"]["logo"].toString();
    data.compact = properties["isCompact"].toBool();
}

void parseGetStoreRankingSectionResponse(const QJsonValue &json, api::GetStoreRankingSectionResponse &data)
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

void parseGetStoreVerticalBannerSectionResponse(const QJsonValue &json, api::GetStoreVerticalBannerSectionResponse &data)
{
    const auto items = json["properties"].toArray();
    data.items.resize(items.count());
    for (std::size_t i = 0; i < items.count(); i++)
    {
        parseVerticalBannerItem(items[i], data.items[i]);
    }
}
