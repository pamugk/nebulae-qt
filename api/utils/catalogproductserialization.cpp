#include "./catalogproductserialization.h"

#include <QJsonArray>
#include <QMap>
#include <QSet>
#include <QVector>

void parseSupportedOS(const QJsonValue &json, api::SupportedOperatingSystem &data)
{
    data.name = json["operatingSystem"]["name"].toString();
    data.versions = json["operatingSystem"]["versions"].toString();

    QSet<QString> definedRequirements;
    auto systemRequirements = json["systemRequirements"].toArray();
    data.systemRequirements.resize(systemRequirements.count());
    for (std::size_t i = 0; i < systemRequirements.count(); i++)
    {
        const QJsonValue &systemRequirementSet = systemRequirements[i];
        data.systemRequirements[i].type = systemRequirementSet["type"].toString();
        data.systemRequirements[i].description = systemRequirementSet["description"].toString();

        auto requirementsList = systemRequirementSet["requirements"].toArray();
        for (const QJsonValue &item : std::as_const(requirementsList))
        {
            auto requirementId = item["id"].toString();
            if (!definedRequirements.contains(requirementId))
            {
                data.definedRequirements.append(api::Requirement{requirementId, item["name"].toString()});
            }
            data.systemRequirements[i].requirements.insert(requirementId, item["description"].toString());
        }
    }
}

void parseRating(const QJsonValue &json, api::ContentRating &data)
{
    data.ageRating = json["ageRating"].toInt();
    auto contentDescriptors = json["contentDescriptors"].toArray();
    data.contentDescriptors.resize(contentDescriptors.count());
    for (const QJsonValue &contentDescriptor : std::as_const(contentDescriptors))
    {
        data.contentDescriptors.append(contentDescriptor["descriptor"].toString());
    }
    data.category = json["category"]["name"].toString();
    data.categoryId = json["category"]["id"].toInteger();
}

void parseBonus(const QJsonValue &json, api::Bonus &data)
{
    data.id = json["id"].toInteger();
    data.name = json["name"].toString();
    data.type.name = json["type"]["name"].toString();
    data.type.slug = json["type"]["slug"].toString();
}

void parseEdition(const QJsonValue &json,
                  QMap<QString, api::Bonus> &bonusMap,
                  api::Edition &data)
{
    data.id = json["id"].toInteger();
    data.name = json["name"].toString();
    auto bonuses = json["bonuses"].toArray();
    data.bonuses.resize(bonuses.count());
    data.bonusSet.reserve(bonuses.count());
    for (std::size_t i = 0; i < bonuses.count(); i++)
    {
        api::Bonus bonus;
        parseBonus(bonuses[i], bonus);
        data.bonuses[i] = bonus.name;
        data.bonusSet.insert(bonus.name);
        if (!bonusMap.contains(bonus.name))
        {
            bonusMap[bonus.name] = bonus;
        }
    }
    data.hasProductCard = json["hasProductCard"].toBool();
    data.link = json["_links"]["self"]["href"].toString();
}

void parseFormattedLink(const QJsonValue &json, api::FormattedLink &data)
{
    data.href = json["href"].toString();
    data.templated = json["templated"].toBool();
    auto formatters = json["formatters"].toArray();
    data.formatters.resize(formatters.count());
    for (const QJsonValue &formatter : std::as_const(formatters))
    {
        data.formatters.append(formatter.toString());
    }
}

void parseLocalizations(const QJsonArray &json, QVector<api::Localization> &data)
{
    data.clear();
    QMap<QString, int> foundLocales;
    for (const QJsonValue &item : json)
    {
        auto locale = item["_embedded"];
        auto language = locale["language"];
        auto languageCode = language["code"].toString();
        if (!foundLocales.contains(languageCode))
        {
            foundLocales.insert(language["code"].toString(), data.count());
            data.append(api::Localization{{languageCode, language["name"].toString()}, false, false});
        }

       auto localizationScope = locale["localizationScope"]["type"].toString();
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

void parseVideo(const QJsonValue &json, api::ThumbnailedVideo &data)
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

void parseCatalogProductInfoResponse(const QJsonValue &json, api::GetCatalogProductInfoResponse &data)
{
    if (json["inDevelopment"].isObject())
    {
        data.inDevelopment = json["inDevelopment"]["active"].toBool();
    }
    data.copyrights = json["copyrights"].toString();
    data.usingDosBox = json["isUsingDosBox"].toBool();
    data.releaseStatus = json["releaseStatus"].toString();
    data.description = json["description"].toString().replace("\\\"", "\"");
    data.size = json["size"].toInt();
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

        const QJsonArray &requiredByGames = links["isRequiredByGames"].toArray();
        for (const QJsonValue &item : requiredByGames)
        {
            data.requiredByGames.append(item["href"].toString());
        }
        const QJsonArray &requiresGames = links["requiresGames"].toArray();
        for (const QJsonValue &item : requiresGames)
        {
            data.requiresGames.append(item["href"].toString());
        }
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
                parseFormattedLink(links["image"], data.imageLink);
                data.checkoutLink = links["checkout"]["href"].toString();
            }
        }
        data.productType = productData["productType"].toString();

        auto localizations = productData["localizations"].toArray();
        parseLocalizations(localizations, data.localizations);

        auto videos = productData["videos"].toArray();
        data.videos.resize(videos.count());
        for (std::size_t i = 0; i < videos.count(); i++)
        {
            parseVideo(videos[i], data.videos[i]);
        }

        auto bonuses = productData["bonuses"].toArray();
        data.bonuses.resize(bonuses.count());
        for (std::size_t i = 0; i < bonuses.count(); i++)
        {
            parseBonus(bonuses[i], data.bonuses[i]);
        }

        auto screenshots = productData["screenshots"].toArray();
        data.screenshots.resize(screenshots.count());
        for (std::size_t i = 0; i < screenshots.count(); i++)
        {
            const QJsonValue &screenshot = screenshots[i];
            parseFormattedLink(screenshot["_links"]["self"], data.screenshots[i]);
        }

        data.publisher = productData["publisher"]["name"].toString();

        auto developers = productData["developers"].toArray();
        data.developers.resize(developers.count());
        for (const QJsonValue &developer : std::as_const(developers))
        {
            data.developers.append(developer["name"].toString());
        }

        auto supportedOperatingSystems = productData["supportedOperatingSystems"].toArray();
        data.supportedOperatingSystems.resize(supportedOperatingSystems.count());
        for (std::size_t i = 0; i < supportedOperatingSystems.count(); i++)
        {
            parseSupportedOS(supportedOperatingSystems[i], data.supportedOperatingSystems[i]);
        }

        auto tags = productData["tags"].toArray();
        data.tags.resize(tags.count());
        for (std::size_t i = 0; i < tags.count(); i++)
        {
            const QJsonValue &tag = tags[i];
            data.tags[i].name = tag["name"].toString();
            data.tags[i].level = tag["level"].toInt();
            data.tags[i].slug = tag["slug"].toString();
        }

        auto properties = productData["properties"].toArray();
        data.properties.resize(properties.count());
        for (std::size_t i = 0; i < properties.count(); i++)
        {
            const QJsonValue &property = properties[i];
            data.properties[i].name = property["name"].toString();
            data.properties[i].slug = property["slug"].toString();
        }

        if (productData["esrbRating"].isObject())
        {
            data.ratings.insert("ESRB", api::ContentRating());
            parseRating(productData["esrbRating"], data.ratings["ESRB"]);
        }
        if (productData["pegiRating"].isObject())
        {
            data.ratings.insert("PEGI", api::ContentRating());
            parseRating(productData["pegiRating"], data.ratings["PEGI"]);
        }
        if (productData["uskRating"].isObject())
        {
            data.ratings.insert("USK", api::ContentRating());
            parseRating(productData["uskRating"], data.ratings["USK"]);
        }
        if (productData["brRating"].isObject())
        {
            data.ratings.insert("BR", api::ContentRating());
            parseRating(productData["brRating"], data.ratings["BR"]);
        }
        if (productData["gogRating"].isObject())
        {
            data.ratings.insert("GOG", api::ContentRating());
            parseRating(productData["gogRating"], data.ratings["GOG"]);
        }

        auto features = productData["features"].toArray();
        data.features.resize(features.count());
        for (std::size_t i = 0; i < features.count(); i++)
        {
            const QJsonValue &feature = features[i];
            data.features[i].name = feature["name"].toString();
            data.features[i].slug = feature["id"].toString();
        }

        auto editions = productData["editions"].toArray();
        data.editions.resize(editions.count());
        for (std::size_t i = 0; i < editions.count(); i++)
        {
            parseEdition(editions[i], data.editionBonuses, data.editions[i]);
        }

        if (productData["series"].isObject())
        {
            auto series = productData["series"];
            data.series.id = series["id"].toInteger();
            data.series.name = series["name"].toString();
        }

        // Merging product bonuses in the following order:
        // common bonuses first, edition-specific bonuses to follow.
        // For some reason, there can be several bonuses with the same name and different ids,
        // so they have to be merged by a name.
        QSet<QString> fullBonusSet;
        for (std::size_t i = 0; i < data.editions.count() - 1; i++)
        {
            for (const QString &bonusName : std::as_const(data.editions[i].bonuses))
            {
                if (fullBonusSet.contains(bonusName))
                {
                    continue;
                }

                bool isCommonBonus = true;
                for (std::size_t j = i + 1; j < data.editions.count() && isCommonBonus; j++)
                {
                    isCommonBonus = data.editions[j].bonusSet.contains(bonusName);
                }

                if (isCommonBonus)
                {
                    fullBonusSet.insert(bonusName);
                    data.fullBonusList.append(bonusName);
                }
            }
        }
        for (std::size_t i = 0; i < data.editions.count(); i++)
        {
            for (const QString &bonusName: std::as_const(data.editions[i].bonuses))
            {
                if (!fullBonusSet.contains(bonusName))
                {
                    fullBonusSet.insert(bonusName);
                    data.fullBonusList.append(bonusName);
                }
            }
        }
    }
}
