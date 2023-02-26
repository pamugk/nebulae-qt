#ifndef PRODUCT_H
#define PRODUCT_H

#include <QtGlobal>

#include <QDate>
#include <QDateTime>
#include <QString>
#include <QVector>

#include "./productshared.h"

namespace api
{
    struct Price
    {
        QString amount;
        QString baseAmount;
        QString finalAmount;
        bool discounted;
        qint32 discountPercentage;
        QString discountDifference;
        QString symbol;
        bool free;
        qreal discount;
        bool bonusStoreCreditIncluded;
        QString bonusStoreCreditAmount;
        QString promoId;
    };

    struct SalesVisibility
    {
        bool active;
        ZonedDateTime fromObject;
        QDateTime from;
        ZonedDateTime toObject;
        QDateTime to;
    };

    struct Video
    {
        QString id;
        QString provider;
    };

    struct Product
    {
        QString developer;
        QString publisher;
        QVector<QString> gallery;
        QVector<QString> supportedOperatingSystems;
        QVector<QString> genres;
        QDate globalReleaseDate;
        qint64 id;
        QDate releaseDate;
        QString title;
        QString image;

        QString url;
        QString supportUrl;
        QString forumUrl;

        QString category;
        QString originalCategory;

        QString slug;
        Video video;

        SalesVisibility salesVisibility;
        Price price;
        qint32 rating;
        bool TBA;
        bool discounted;
        bool inDevelopment;
        bool buyable;
        qint8 type;
        bool comingSoon;
        bool priceVisible;
        bool movie;
        bool game;
        bool wishlistable;
        quint8 ageLimit;
        Availability availability;
        PlatformAvailability worksOn;
    };
}

#endif // PRODUCT_H
