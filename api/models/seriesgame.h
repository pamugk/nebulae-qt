#ifndef SERIESGAME_H
#define SERIESGAME_H

#include <QVector>

namespace api
{
    struct SeriesGame
    {
        unsigned long long id;
        QString title;
        QString releaseStatus;
        QString productType;
        QString imageLink;

        bool inDevelopment;
        bool availableForSale;
        bool visibleInCatalog;
        bool preorder;
        bool visibleInAccount;
        bool installable;
        bool hasProductCard;
        bool secret;
    };

    struct GetSeriesGamesResponse
    {
        QVector<SeriesGame> items;
        unsigned char limit;
        unsigned short page;
        unsigned short pages;
    };
}

#endif // SERIESGAME_H
