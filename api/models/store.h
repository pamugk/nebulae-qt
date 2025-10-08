#ifndef STORE_H
#define STORE_H

#include <QString>
#include <QVector>

#include "catalog.h"

namespace api
{
    struct StoreSection
    {
        QString id;
        QString sectionType;
        bool personalized;
        bool hideOnLoad;
        bool loadOnEmbed;
        QString contentSourceType;
    };

    struct GetStoreProductsSectionResponse
    {
        QString sectionId;
        QString contentSourceType;
        QString title;
        QString description;
        QVector<CatalogProduct> items;
        QString seeMoreLink;
    };

    struct GetStoreSectionsResponse
    {
        QVector<StoreSection> sections;
    };
}

#endif // STORE_H
