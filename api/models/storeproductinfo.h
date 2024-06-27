#ifndef STOREPRODUCTINFO_H
#define STOREPRODUCTINFO_H

#include <QDateTime>
#include <QMap>
#include <QString>
#include <QVector>

namespace api
    {
    struct DownloadFile
    {
        QString id;
        unsigned int size;
        QString downloadLink;
    };

    struct Download
    {
        QString id;
        QString name;
        unsigned int totalSize;
        QVector<DownloadFile> files;
    };

    struct BonusDownload: Download
    {
        QString type;
        unsigned char count;
    };

    struct GameDownload: Download
    {
        QString os;
        QString language;
        QString languageFull;
        QString version;
    };

    struct Downloads
    {
        QVector<GameDownload> installers;
        QVector<GameDownload> patches;
        QVector<GameDownload> languagePacks;
        QVector<BonusDownload> bonusContent;
    };

    struct ProductInfo
    {
        unsigned long long id;
        QString slug;
        QString title;
        QMap<QString, QString> languages;
        QString purchaseLink;
        QString productCardLink;
        QString supportLink;
        QString forumLink;
        bool inDevelopment;
        QDateTime developedUntil;
        bool secret;
        bool installable;
        QString gameType;
        bool preOrder;
        QDateTime releaseDate;
        QMap<QString, QString> images;
        Downloads downloads;
    };

    struct ProductScreenshot
    {
        QString imageId;
        QString formattedTemplateUrl;
        QMap<QString, QString> formattedImages;
    };

    struct ProductVideo
    {
        QString videoUrl;
        QString thumbnailUrl;
        QString provider;
    };

    struct GetStoreProductInfoResponse
    {
        ProductInfo mainProductInfo;
        QVector<ProductInfo> expandedDlcs;
        QVector<ProductInfo> relatedProducts;
        QVector<ProductScreenshot> screenshots;
        QVector<ProductVideo> videos;
        QString descriptionLead;
        QString descriptionFull;
        QString changelog;
    };
}

#endif // STOREPRODUCTINFO_H
