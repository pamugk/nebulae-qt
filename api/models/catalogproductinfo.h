#ifndef CATALOGPRODUCTINFO_H
#define CATALOGPRODUCTINFO_H

#include <QtGlobal>

#include <QDateTime>
#include <QMap>
#include <QSet>
#include <QString>
#include <QStringList>
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
        QVector<QString> bonuses;
        QSet<QString> bonusSet;
    };

    struct ContentRating
    {
        quint8 ageRating;
        QString category;
        QStringList contentDescriptors;
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
        quint64 id;
        QDateTime globalReleaseDate;
        QDateTime gogReleaseDate;
        QMap<QString, ContentRating> ratings;
        QString copyrights;
        QString releaseStatus;
        QString description;
        QString overview;
        QString featuresDescription;
        QString additionalRequirements;
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
        QString title;
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
        QVector<MetaTag> features;
        QMap<QString, Bonus> editionBonuses;
        QVector<QString> fullBonusList;
        QVector<Edition> editions;
        Series series;
        FormattedLink imageLink;
        quint32 size;

        bool inDevelopment;
        bool usingDosBox;
        bool availableForSale;
        bool visibleInCatalog;
        bool preorder;
        bool visibleInAccount;
        bool installable;
        bool hasProductCard;
        bool secret;
    };
}

#endif // CATALOGPRODUCTINFO_H
