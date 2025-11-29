#include "./preservationprogramserialization.h"

#include <QJsonArray>

#include "./catalogserialization.h"

void parseAccordionItem(const QJsonValue &json, api::AccordionSectionItem &data)
{
    data.header = json[QLatin1StringView("header")].toString();
    data.description = json[QLatin1StringView("description")].toString();
}

void parsePreservationStatsItem(const QJsonValue &json, api::PreservationStatsItem &data)
{
    data.icon = json[QLatin1StringView("icon")].toString();
    data.name = json[QLatin1StringView("name")].toString();
    data.value = json[QLatin1StringView("value")].toString();
    data.suffix = json[QLatin1StringView("suffix")].toString();
}

void parsePreservationProgramStoryItem(const QJsonValue &json, api::PreservationProgramStory &data)
{
    parseCatalogProduct(json[QLatin1StringView("product")], data.product, QLatin1StringView("_gog_pp_story_480w.webp"));
    data.description = json[QLatin1StringView("description")].toString();
    data.ctaButton.copy = json[QLatin1StringView("ctaButton")][QLatin1StringView("copy")].toString();
    data.ctaButton.href = json[QLatin1StringView("ctaButton")][QLatin1StringView("href")].toString();
    data.changelogButtonText = json[QLatin1StringView("changelogButtonText")].toString();
    const QJsonArray changelogItems = json[QLatin1StringView("changelogItems")].toArray();
    data.changelogItems.resize(changelogItems.count());
    for (std::size_t i = 0; i < changelogItems.count(); i++)
    {
        parseAccordionItem(changelogItems[i], data.changelogItems[i]);
    }
}

void parseGetAccordionSectionResponse(const QJsonValue &json, api::GetAccordionSectionResponse &data)
{
    const auto properties = json[QLatin1StringView("properties")];
    data.title = properties[QLatin1StringView("title")].toString();
    const QJsonArray items = properties[QLatin1StringView("items")].toArray();
    data.items.resize(items.count());
    for (std::size_t i = 0; i < items.count(); i++)
    {
        parseAccordionItem(items[i], data.items[i]);
    }
}

void parseGetPreservationProgramDescriptionSectionResponse(const QJsonValue &json, api::GetPreservationProgramDescriptionSectionResponse &data)
{
    const auto properties = json[QLatin1StringView("properties")];
    data.description = properties[QLatin1StringView("description")].toString();
    data.button.copy = properties[QLatin1StringView("button")][QLatin1StringView("copy")].toString();
    data.button.href = properties[QLatin1StringView("button")][QLatin1StringView("href")].toString();
}

void parseGetPreservationProgramHeroSectionResponse(const QJsonValue &json, api::GetPreservationProgramHeroSectionResponse &data)
{
    const auto properties = json[QLatin1StringView("properties")];
    data.title = properties[QLatin1StringView("title")].toString();
    data.subtitle = properties[QLatin1StringView("subtitle")].toString();
    const QJsonArray preservationStats = properties[QLatin1StringView("preservationStats")].toArray();
    data.preservationStats.resize(preservationStats.count());
    for (std::size_t i = 0; i < preservationStats.count(); i++)
    {
        parsePreservationStatsItem(preservationStats[i], data.preservationStats[i]);
    }
    const QJsonArray games = properties[QLatin1StringView("games")].toArray();
    data.games.resize(games.count());
    for (std::size_t i = 0; i < games.count(); i++)
    {
        parseCatalogProduct(games[i], data.games[i], QLatin1StringView("_gog_pp_story_480w.webp"));
    }
    data.ctaButton.copy = properties[QLatin1StringView("ctaButton")][QLatin1StringView("copy")].toString();
    data.ctaButton.href = properties[QLatin1StringView("ctaButton")][QLatin1StringView("href")].toString();
    data.videoButton.copy = properties[QLatin1StringView("videoButton")][QLatin1StringView("copy")].toString();
    data.videoButton.href = properties[QLatin1StringView("videoButton")][QLatin1StringView("href")].toString();
}

void parseGetPreservationProgramStoriesSectionResponse(const QJsonValue &json, api::GetPreservationProgramStoriesSectionResponse &data)
{
    const auto properties = json[QLatin1StringView("properties")];
    data.title = properties[QLatin1StringView("title")].toString();
    data.seeMoreText = properties[QLatin1StringView("seeMoreCopy")].toString();
    data.seeMoreLink = properties[QLatin1StringView("seeMoreLink")].toString();
    data.shouldDisplaySeeMoreIcon = properties[QLatin1StringView("shouldDisplaySeeMoreIcon")].toBool();
    const QJsonArray stories = properties[QLatin1StringView("stories")].toArray();
    data.stories.resize(stories.count());
    for (std::size_t i = 0; i < stories.count(); i++)
    {
        parsePreservationProgramStoryItem(stories[i], data.stories[i]);
    }
}
