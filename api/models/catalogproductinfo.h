#ifndef CATALOGPRODUCTINFO_H
#define CATALOGPRODUCTINFO_H

#include <QtGlobal>

#include <QDateTime>
#include <QMap>
#include <QString>
#include <QVector>

#include "./metatag.h"

namespace api
{
    struct Bonus
    {
        quint64 id;
        QString name;
        MetaTag type;
    };

    struct Edition
    {
        quint64 id;
        QString name;
        QString link;
        bool hasProductCard;
        QVector<Bonus> bonuses;
    };

    struct ContentRating
    {
        quint8 ageRating;
        QString category;
        QVector<QString> contentDescriptors;
    };

    struct Requirement
    {
        QString id;
        QString name;
    };

    struct SystemRequirements
    {
        QString type;
        QString description;
        QMap<QString, QString> requirements;
    };

    struct SupportedOperatingSystem
    {
        QString name;
        QString versions;
        QVector<SystemRequirements> systemRequirements;
        QVector<Requirement> definedRequirements;
    };

    struct ThumbnailedVideo
    {
        QString provider;
        QString videoId;
        QString thumbnailId;
        QString videoLink;
        QString thumbnailLink;
    };

    struct Localization
    {
        struct
        {
            QString code;
            QString name;
        } language;
        bool localizedText;
        bool localizedAuio;
    };

    struct FormattedLink
    {
        QString href;
        bool templated;
        QVector<QString> formatters;
    };

    struct Series
    {
        quint64 id;
        QString name;
    };

    struct GetCatalogProductInfoResponse
    {
        // General info
        bool inDevelopment;
        bool usingDosBox;
        QString copyrights;
        QString releaseStatus;
        QString description;
        QString overview;
        QString featuresDescription;
        QString additionalRequirements;

        // Links
        QString storeLink;
        QString supportLink;
        QString forumLink;
        QString iconLink;
        QString iconSquareLink;
        QString logoLink;
        QString boxArtImageLink;
        QString backgroundImageLink;
        QString galaxyBackgroundImageLink;
        QVector<QString> includedGames;

        // Main product info
        bool availableForSale;
        bool visibleInCatalog;
        bool preorder;
        bool visibleInAccount;
        quint64 id;
        QString title;
        bool installable;
        QDateTime globalReleaseDate;
        bool hasProductCard;
        QDateTime gogReleaseDate;
        bool secret;
        FormattedLink imageLink;
        QString checkoutLink;
        QString productType;
        QVector<Localization> localizations;
        QVector<ThumbnailedVideo> videos;
        QVector<Bonus> bonuses;
        QVector<FormattedLink> screenshots;
        QString publisher;
        QVector<QString> developers;
        QVector<SupportedOperatingSystem> supportedOperatingSystems;
        QVector<HierarchicalMetaTag> tags;
        QVector<MetaTag> properties;
        QMap<QString, ContentRating> ratings;
        QVector<MetaTag> features;
        QVector<Edition> editions;
        Series series;
    };
}

#endif // CATALOGPRODUCTINFO_H
