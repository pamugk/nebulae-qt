#include "./serialization.h"

#include <QJsonArray>

void parseZonedDateTime(const QJsonValue &json, api::ZonedDateTime &data)
{
    data.date = QDateTime::fromString(json["date"].toString(), Qt::ISODateWithMs);
    data.timezoneType = json["timezone_type"].toInt();
    data.timezone = json["timezone"].toString();
}

void parseProductData(const QJsonValue &json, api::Product &data)
{
    data.developer = json["developer"].toString();
    data.publisher = json["publisher"].toString();

    auto gallery = json["gallery"].toArray();
    data.gallery.resize(gallery.count());
    for (std::size_t i = 0; i < gallery.count(); i++)
    {
        data.gallery[i] = gallery[i].toString();
    }

    if (json["video"].isObject())
    {
        auto video = json["video"];
        data.video.id = video["id"].toString();
        data.video.provider = video["provider"].toString();
    }

    auto supportedOperatingSystems = json["supportedOperatingSystems"].toArray();
    data.supportedOperatingSystems.resize(supportedOperatingSystems.count());
    for (std::size_t i = 0; i < supportedOperatingSystems.count(); i++)
    {
        data.supportedOperatingSystems[i] = supportedOperatingSystems[i].toString();
    }

    auto genres = json["genres"].toArray();
    data.genres.resize(genres.count());
    for (std::size_t i = 0; i < genres.count(); i++)
    {
        data.genres[i] = genres[i].toString();
    }

    if (!json["globalReleaseDate"].isNull())
    {
        data.globalReleaseDate = QDateTime::fromSecsSinceEpoch(json["globalReleaseDate"].toInt()).date();
    }
    data.TBA = json["isTBA"].toBool();
    if (json["price"].isObject())
    {
        auto price = json["price"];
        data.price.amount = price["amount"].toString();
        data.price.baseAmount = price["baseAmount"].toString();
        data.price.finalAmount = price["finalAmount"].toString();
        data.price.discounted = price["isDiscounted"].toBool();
        data.price.discountPercentage = price["discountPercentage"].toInt();
        data.price.discountDifference = price["discountDifference"].toString();
        data.price.symbol = price["symbol"].toString();
        data.price.free = price["isFree"].toBool();
        data.price.discount = price["discount"].toInt();
        data.price.bonusStoreCreditIncluded = price["isBonusStoreCreditIncluded"].toBool();
        data.price.bonusStoreCreditAmount = price["bonusStoreCreditAmount"].toString();
    }
    data.discounted = json["isDiscounted"].toBool();
    data.inDevelopment = json["isInDevelopment"].toBool();
    data.id = json["id"].toInteger();
    if (!json["releaseDate"].isNull())
    {
        data.releaseDate = QDateTime::fromSecsSinceEpoch(json["releaseDate"].toInt()).date();
    }
    if (json["availability"].isObject())
    {
        auto availability = json["availability"];
        data.availability.available = availability["isAvailable"].toBool();
        data.availability.availableInAccount = availability["isAvailableInAccount"].toBool();
    }
    if (json["salesVisibility"].isObject())
    {
        auto salesVisibility = json["salesVisibility"];
        data.salesVisibility.active = salesVisibility["isActive"].toBool();
        parseZonedDateTime(salesVisibility["fromObject"], data.salesVisibility.fromObject);
        data.salesVisibility.from = QDateTime::fromSecsSinceEpoch(salesVisibility["from"].toInt());
        parseZonedDateTime(salesVisibility["toObject"], data.salesVisibility.toObject);
        data.salesVisibility.to = QDateTime::fromSecsSinceEpoch(salesVisibility["to"].toInt());
    }
    data.buyable = json["buyable"].toBool();
    data.title = json["title"].toString();
    data.image = json["image"].toString();
    data.url = json["url"].toString();
    data.supportUrl = json["supportUrl"].toString();
    data.forumUrl = json["forumUrl"].toString();
    if (json["worksOn"].isObject())
    {
        auto worksOn = json["worksOn"];
        data.worksOn.Windows = worksOn["Windows"].toBool();
        data.worksOn.Mac = worksOn["Mac"].toBool();
        data.worksOn.Linux = worksOn["Linux"].toBool();
    }
    data.category = json["category"].toString();
    data.originalCategory = json["originalCategory"].toString();
    data.rating = json["rating"].toInt();
    data.type = json["type"].toInt();
    data.comingSoon = json["isComingSoon"].toBool();
    data.priceVisible = json["isPriceVisible"].toBool();
    data.movie = json["isMovie"].toBool();
    data.game = json["isGame"].toBool();
    data.slug = json["slug"].toString();
    data.wishlistable = json["isWishlistable"].toBool();
    data.ageLimit = json["ageLimit"].toInt();
}

void parseSearchWishlistResponse(const QJsonValue &json, api::GetWishlistResponse &data)
{
    data.page = json["page"].toInt(1);
    data.totalProducts = json["totalProducts"].toInt(0);
    data.totalPages = json["totalPages"].toInt(0);
    data.productsPerPage = json["productsPerPage"].toInt(0);
    auto products = json["products"].toArray();
    data.products.resize(products.count());

    for (std::size_t i = 0; i < products.count(); i++)
    {
        parseProductData(products[i], data.products[i]);
    }
}
