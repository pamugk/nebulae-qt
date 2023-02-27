#include "./catalogproductserialization.h"

#include <QJsonArray>
#include <QMap>
#include <QSet>
#include <QVector>

void parseSupportedOS(const QJsonObject &json, api::SupportedOperatingSystem &data)
{
    data.name = json["operatingSystem"]["name"].toString();
    data.versions = json["operatingSystem"]["versions"].toString();

    QSet<QString> definedRequirements;
    auto systemRequirements = json["systemRequirements"].toArray();
    data.systemRequirements.resize(systemRequirements.count());
    for (int i = 0; i < systemRequirements.count(); i++)
    {
        auto systemRequirementSet = systemRequirements[i].toObject();
        data.systemRequirements[i].type = systemRequirementSet["type"].toString();
        data.systemRequirements[i].description = systemRequirementSet["description"].toString();

        auto requirementsList = systemRequirementSet["requirements"].toArray();
        for (int j = 0; j < requirementsList.count(); j++)
        {
            auto requirement = requirementsList[j].toObject();
            auto requirementId = requirement["id"].toString();
            if (!definedRequirements.contains(requirementId))
            {
                data.definedRequirements.append(api::Requirement{requirementId, requirement["name"].toString()});
            }
            data.systemRequirements[i].requirements.insert(requirementId, requirement["description"].toString());
        }
    }
}

void parseRating(const QJsonObject &json, api::ContentRating &data)
{
    data.ageRating = json["ageRating"].toInt();
    auto contentDescriptors = json["contentDescriptors"].toArray();
    data.contentDescriptors.resize(contentDescriptors.count());
    for (int i = 0; i < contentDescriptors.count(); i++)
    {
        data.contentDescriptors[i] = contentDescriptors[i].toObject()["descriptor"].toString();
    }
    data.category = json["category"]["name"].toString();
}

void parseBonus(const QJsonObject &json, api::Bonus &data)
{
    data.id = json["id"].toInteger();
    data.name = json["name"].toString();
    data.type.name = json["type"]["name"].toString();
    data.type.slug = json["type"]["slug"].toString();
}

void parseEdition(const QJsonObject &json, api::Edition &data)
{
    data.id = json["id"].toInteger();
    data.name = json["name"].toString();
    auto bonuses = json["bonuses"].toArray();
    data.bonuses.resize(bonuses.count());
    for (int i = 0; i < bonuses.count(); i++)
    {
        parseBonus(bonuses[i].toObject(), data.bonuses[i]);
    }
    data.hasProductCard = json["hasProductCard"].toBool();
    data.link = json["_links"]["self"]["href"].toString();
}

void parseFormattedLink(const QJsonObject &json, api::FormattedLink &data)
{
    data.href = json["href"].toString();
    data.templated = json["templated"].toBool();
    auto formatters = json["formatters"].toArray();
    data.formatters.resize(formatters.count());
    for (int i = 0; i < formatters.count(); i++)
    {
        data.formatters[i] = formatters[i].toString();
    }
}

void parseLocalizations(const QJsonArray &json, QVector<api::Localization> &data)
{
    data.clear();
    QMap<QString, int> foundLocales;
    for (int i = 0; i < json.count(); i++)
    {
        auto locale = json[i].toObject()["_embedded"].toObject();
        auto language = locale["language"].toObject();
        auto languageCode = language["code"].toString();
        if (!foundLocales.contains(languageCode))
        {
            foundLocales.insert(language["code"].toString(), data.count());
            data.append(api::Localization{{languageCode, language["name"].toString()}, false, false});
        }

       auto localizationScope = locale["localizationScope"].toObject()["type"].toString();
       if (localizationScope == "text")
       {
           data[foundLocales[languageCode]].localizedText = true;
       }
       else if (localizationScope == "audio")
       {
           data[foundLocales[languageCode]].localizedAuio = true;
       }
    }
}

void parseVideo(const QJsonObject &json, api::ThumbnailedVideo &data)
{
    data.provider = json["provider"].toString();
    data.videoId = json["videoId"].toString();
    data.thumbnailId = json["thumbnailId"].toString();
    if (json["_links"].isObject())
    {
        auto links = json["_links"];
        data.videoLink = links["self"]["href"].toString();
        data.thumbnailLink = links["thumbnail"]["href"].toString();
    }
}

void parseCatalogProductInfoResponse(const QJsonObject &json, api::GetCatalogProductInfoResponse &data)
{
    if (json["inDevelopment"].isObject())
    {
        data.inDevelopment = json["inDevelopment"]["active"].toBool();
    }
    data.copyrights = json["copyrights"].toString();
    data.usingDosBox = json["isUsingDosBox"].toBool();
    data.releaseStatus = json["releaseStatus"].toString();
    data.description = json["description"].toString().replace("\\\"", "\"");
    data.overview = json["overview"].toString().replace("\\\"", "\"");
    data.featuresDescription = json["featuresDescription"].toString();
    data.additionalRequirements = json["additionalRequirements"].toString();

    if (json["_links"].isObject())
    {
        auto links = json["_links"];
        data.storeLink = links["store"]["href"].toString();
        data.supportLink = links["support"]["href"].toString();
        data.forumLink = links["forum"]["href"].toString();
        data.iconLink = links["icon"]["href"].toString();
        data.iconSquareLink = links["iconSquare"]["href"].toString();
        data.logoLink = links["logo"]["href"].toString();
        data.boxArtImageLink = links["boxArtImage"]["href"].toString();
        data.backgroundImageLink = links["backgroundImage"]["href"].toString();
        data.galaxyBackgroundImageLink = links["galaxyBackgroundImage"]["href"].toString();
    }

    if (json["_embedded"].isObject())
    {
        auto productData = json["_embedded"];
        if (productData["product"].isObject())
        {
            auto product = productData["product"];
            data.availableForSale = product["isAvailableForSale"].toBool();
            data.visibleInCatalog = product["isVisibleInCatalog"].toBool();
            data.preorder = product["isPreorder"].toBool();
            data.visibleInAccount = product["isVisibleInAccount"].toBool();
            data.id = product["id"].toInteger();
            data.title = product["title"].toString();
            data.installable = product["isInstallable"].toBool();
            if (product["globalReleaseDate"].isString())
            {
                data.globalReleaseDate = QDateTime::fromString(product["globalReleaseDate"].toString(), Qt::DateFormat::ISODate);
            }
            data.hasProductCard = product["hasProductCard"].toBool();
            if (product["gogReleaseDate"].isString())
            {
                data.gogReleaseDate = QDateTime::fromString(product["gogReleaseDate"].toString(), Qt::DateFormat::ISODate);
            }
            data.secret = product["isSecret"].toBool();

            if (product["_links"].isObject())
            {
                auto links = product["_links"];
                parseFormattedLink(links["image"].toObject(), data.imageLink);
                data.checkoutLink = links["checkout"]["href"].toString();
            }
        }
        data.productType = productData["productType"].toString();

        auto localizations = productData["localizations"].toArray();
        parseLocalizations(localizations, data.localizations);

        auto videos = productData["videos"].toArray();
        data.videos.resize(videos.count());
        for (int i = 0; i < videos.count(); i++)
        {
            parseVideo(videos[i].toObject(), data.videos[i]);
        }

        auto bonuses = productData["bonuses"].toArray();
        data.bonuses.resize(bonuses.count());
        for (int i = 0; i < bonuses.count(); i++)
        {
            parseBonus(bonuses[i].toObject(), data.bonuses[i]);
        }

        auto screenshots = productData["screenshots"].toArray();
        data.screenshots.resize(screenshots.count());
        for (int i = 0; i < screenshots.count(); i++)
        {
            parseFormattedLink(screenshots[i].toObject()["_links"].toObject()["self"].toObject(), data.screenshots[i]);
        }

        data.publisher = productData["publisher"]["name"].toString();

        auto developers = productData["developers"].toArray();
        data.developers.resize(developers.count());
        for (int i = 0; i < developers.count(); i++)
        {
            data.developers[i] = developers[i].toObject()["name"].toString();
        }

        auto supportedOperatingSystems = productData["supportedOperatingSystems"].toArray();
        data.supportedOperatingSystems.resize(supportedOperatingSystems.count());
        for (int i = 0; i < supportedOperatingSystems.count(); i++)
        {
            parseSupportedOS(supportedOperatingSystems[i].toObject(), data.supportedOperatingSystems[i]);
        }

        auto tags = productData["tags"].toArray();
        data.tags.resize(tags.count());
        for (int i = 0; i < tags.count(); i++)
        {
            auto tag = tags[i].toObject();
            data.tags[i].name = tag["name"].toString();
            data.tags[i].level = tag["level"].toInt();
            data.tags[i].slug = tag["slug"].toString();
        }

        auto properties = productData["properties"].toArray();
        data.properties.resize(properties.count());
        for (int i = 0; i < properties.count(); i++)
        {
            auto property = properties[i].toObject();
            data.properties[i].name = property["name"].toString();
            data.properties[i].slug = property["slug"].toString();
        }

        if (productData["esrbRating"].isObject())
        {
            data.ratings.insert("ESRB", api::ContentRating());
            parseRating(productData["esrbRating"].toObject(), data.ratings["ESRB"]);
        }
        if (productData["pegiRating"].isObject())
        {
            data.ratings.insert("PEGI", api::ContentRating());
            parseRating(productData["pegiRating"].toObject(), data.ratings["PEGI"]);
        }
        if (productData["uskRating"].isObject())
        {
            data.ratings.insert("USK", api::ContentRating());
            parseRating(productData["uskRating"].toObject(), data.ratings["USK"]);
        }
        if (productData["brRating"].isObject())
        {
            data.ratings.insert("BR", api::ContentRating());
            parseRating(productData["brRating"].toObject(), data.ratings["BR"]);
        }
        if (productData["gogRating"].isObject())
        {
            data.ratings.insert("GOG", api::ContentRating());
            parseRating(productData["gogRating"].toObject(), data.ratings["GOG"]);
        }

        auto features = productData["features"].toArray();
        data.features.resize(features.count());
        for (int i = 0; i < features.count(); i++)
        {
            auto feature = features[i].toObject();
            data.features[i].name = feature["name"].toString();
            data.features[i].slug = feature["id"].toString();
        }

        auto editions = productData["editions"].toArray();
        data.editions.resize(editions.count());
        for (int i = 0; i < editions.count(); i++)
        {
            parseEdition(editions[i].toObject(), data.editions[i]);
        }

        if (productData["series"].isObject())
        {
            auto series = productData["series"];
            data.series.id = series["id"].toInteger();
            data.series.name = series["name"].toString();
        }
    }
}
