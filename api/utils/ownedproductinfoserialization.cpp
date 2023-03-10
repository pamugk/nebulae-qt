#include "./ownedproductinfoserialization.h"

#include <QJsonArray>

void parseDownloadFile(const QJsonObject &json, api::DownloadFile &data)
{
    if (json["id"].isString())
    {
        data.id = json["id"].toString();
    }
    else
    {
        data.id = QString::number(json["id"].toInteger());
    }
    data.size = json["size"].toInt();
    data.downloadLink = json["downlink"].toString();
}

void parseDownload(const QJsonObject &json, api::Download &data)
{
    if (json["id"].isString())
    {
        data.id = json["id"].toString();
    }
    else
    {
        data.id = QString::number(json["id"].toInteger());
    }
    data.name = json["name"].toString();
    data.totalSize = json["total_size"].toInt();
    auto files = json["files"].toArray();
    data.files.resize(files.count());
    for (int i = 0; i < files.count(); i++)
    {
        parseDownloadFile(files[i].toObject(), data.files[i]);
    }
}

void parseBonusDownload(const QJsonObject &json, api::BonusDownload &data)
{
    parseDownload(json, data);
    data.type = json["type"].toString();
    data.count = json["count"].toInt();
}


void parseGameDownload(const QJsonObject &json, api::GameDownload &data)
{
    parseDownload(json, data);
    data.os = json["os"].toString();
    data.language = json["language"].toString();
    data.languageFull = json["language_full"].toString();
    data.version = json["version"].toString();
}

void parseDownloads(const QJsonObject &json, api::Downloads &data)
{
    QJsonArray downloads;
    downloads = json["installers"].toArray();
    data.installers.resize(downloads.count());
    for (int i = 0; i < downloads.count(); i++)
    {
        parseGameDownload(downloads[i].toObject(), data.installers[i]);
    }

    downloads = json["patches"].toArray();
    data.patches.resize(downloads.count());
    for (int i = 0; i < downloads.count(); i++)
    {
        parseGameDownload(downloads[i].toObject(), data.patches[i]);
    }

    downloads = json["language_packs"].toArray();
    data.languagePacks.resize(downloads.count());
    for (int i = 0; i < downloads.count(); i++)
    {
        parseGameDownload(downloads[i].toObject(), data.languagePacks[i]);
    }

    downloads = json["bonus_content"].toArray();
    data.bonusContent.resize(downloads.count());
    for (int i = 0; i < downloads.count(); i++)
    {
        parseBonusDownload(downloads[i].toObject(), data.bonusContent[i]);
    }
}

void parseProductInfo(const QJsonObject &json, api::ProductInfo &data)
{
    data.id = json["id"].toInteger();
    data.title = json["title"].toString();
    data.slug = json["slug"].toString();
    auto languages = json["languages"].toVariant().toMap();
    foreach (QString language, languages.keys())
    {
        data.languages[language] = languages[language].toString();
    }
    data.purchaseLink = json["links"]["purchase_link"].toString();
    data.productCardLink = json["links"]["product_card"].toString();
    data.supportLink = json["links"]["support"].toString();
    data.forumLink = json["links"]["forum"].toString();
    data.inDevelopment = json["in_development"]["active"].toBool();
    if (json["in_development"]["until"].isString())
    {
        data.developedUntil = QDateTime::fromString(json["in_development"]["until"].toString(), Qt::ISODate);
    }
    data.secret = json["is_secret"].toBool();
    data.installable = json["is_installable"].toBool();
    data.gameType = json["game_type"].toString();
    data.preOrder = json["is_pre_order"].toBool();
    data.releaseDate = QDateTime::fromString(json["release_date"].toString(), Qt::ISODate);
    auto images = json["images"].toVariant().toMap();
    foreach (QString image, images.keys())
    {
        data.images[image] = images[image].toString();
    }
    parseDownloads(json["downloads"].toObject(), data.downloads);
}

void parseProductScreenshot(const QJsonObject &json, api::ProductScreenshot &data)
{
    data.imageId = json["image_id"].toString();
    data.formattedTemplateUrl = json["formatter_template_url"].toString();
    data.formattedImages.clear();
    if (json["formatted_images"].isArray())
    {
        auto images = json["formatted_images"].toArray();
        foreach (QJsonValue image, images)
        {
            data.formattedImages[image["formatter_name"].toString()] = image["image_url"].toString();
        }
    }
}

void parseProductVideo(const QJsonObject &json, api::ProductVideo &data)
{
    data.videoUrl = json["video_url"].toString();
    data.thumbnailUrl = json["thumbnail_url"].toString();
    data.provider = json["provider"].toString();
}

void parseGetOwnedProductInfoResponse(const QJsonObject &json, api::GetOwnedProductInfoResponse &data)
{
    parseProductInfo(json, data.mainProductInfo);
    auto dlcs = json["expanded_dlcs"].toArray();
    data.expandedDlcs.resize(dlcs.count());
    for (int i = 0; i < dlcs.count(); i++)
    {
        parseProductInfo(dlcs[i].toObject(), data.expandedDlcs[i]);
    }
    auto relatedProducts = json["related_products"].toArray();
    data.relatedProducts.resize(relatedProducts.count());
    for (int i = 0; i < relatedProducts.count(); i++)
    {
        parseProductInfo(relatedProducts[i].toObject(), data.relatedProducts[i]);
    }

    auto screenshots = json["videos"].toArray();
    data.screenshots.resize(screenshots.count());
    for (int i = 0; i < screenshots.count(); i++)
    {
        parseProductScreenshot(screenshots[i].toObject(), data.screenshots[i]);
    }

    auto videos = json["videos"].toArray();
    data.videos.resize(videos.count());
    for (int i = 0; i < videos.count(); i++)
    {
        parseProductVideo(videos[i].toObject(), data.videos[i]);
    }

    data.descriptionLead = json["description"]["lead"].toString();
    data.descriptionFull = json["description"]["full"].toString();
    data.changelog = json["changelog"].toString();
}
