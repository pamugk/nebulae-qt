#ifndef PRODUCT_H
#define PRODUCT_H

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
        int discountPercentage;
        QString discountDifference;
        QString symbol;
        bool free;
        double discount;
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
        long long id;
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
        int rating;
        bool TBA;
        bool discounted;
        bool inDevelopment;
        bool buyable;
        signed char type;
        bool comingSoon;
        bool priceVisible;
        bool movie;
        bool game;
        bool wishlistable;
        signed char ageLimit;
        Availability availability;
        PlatformAvailability worksOn;
    };
}

#endif // PRODUCT_H
