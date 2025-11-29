#include <QString>
#include <QVector>

#include "./catalog.h"

#ifndef PRESERVATIONPROGRAM_H
#define PRESERVATIONPROGRAM_H
namespace api
{
    struct AccordionSectionItem
    {
        QString header;
        QString description;
    };

    struct PreservationProgramStory
    {
        CatalogProduct product;
        QString description;
        struct
        {
            QString copy;
            QString href;
        } ctaButton;
        QString changelogButtonText;
        QVector<AccordionSectionItem> changelogItems;
    };

    struct PreservationStatsItem
    {
        QString icon;
        QString name;
        QString value;
        QString suffix;
    };

    struct GetAccordionSectionResponse
    {
        QString title;
        QVector<AccordionSectionItem> items;
    };

    struct GetPreservationProgramDescriptionSectionResponse
    {
        QString title;
        QString description;
        struct
        {
            QString copy;
            QString href;
        } button;
    };

    struct GetPreservationProgramHeroSectionResponse
    {
        QString title;
        QString subtitle;
        QVector<PreservationStatsItem> preservationStats;
        QVector<CatalogProduct> games;
        struct
        {
            QString copy;
            QString href;
        } ctaButton;
        struct
        {
            QString copy;
            QString href;
        } videoButton;
    };

    struct GetPreservationProgramStoriesSectionResponse
    {
        QString title;
        QString seeMoreText;
        QString seeMoreLink;
        bool shouldDisplaySeeMoreIcon;
        QVector<PreservationProgramStory> stories;
    };
}

#endif // PRESERVATIONPROGRAM_H
