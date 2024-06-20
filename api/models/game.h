#ifndef GAME_H
#define GAME_H

#include <QDateTime>
#include <QString>
#include <QVector>

namespace api
{
    struct IdMetaTag
    {
        QString id;
        QString slug;
        QString name;
    };

    struct LocalizedMetaTag
    {
        QString id;
        QMap<QString, QString> name;
        QString slug;
    };

    struct NamedVideo
    {
        QString provider;
        QString videoId;
        QString thumbnailId;
        QString name;
    };

    struct PlatformRelease
    {
        QString id;
        QString platformId;
        QString externalId;
        QString releasePerPlatformId;
    };

    struct Game
    {
        QString id;
        QString parentId;
        QVector<QString> dlcsIds;
        QDateTime firstReleaseDate;
        QVector<PlatformRelease> releases;
        QMap<QString, QString> title;
        QMap<QString, QString> sortingTitle;
        QString type;
        QVector<QString> developersIds;
        QVector<IdMetaTag> developers;
        QVector<QString> publishersIds;
        QVector<IdMetaTag> publishers;
        QVector<QString> genresIds;
        QVector<LocalizedMetaTag> genres;
        QVector<QString> themesIds;
        QVector<QString> screenshots;
        QVector<NamedVideo> videos;
        QVector<QString> artworks;
        QMap<QString, QString> summary;
        bool visibleInLibrary;
        double aggregatedRating;
        QVector<IdMetaTag> gameModes;
        QString horizontalArtwork;
        QString background;
        QString verticalCover;
        QString cover;
        QString logo;
        QString squareIcon;
        int globalPopularityAllTime;
        int globalPopularityCurrent;
    };
}

#endif // GAME_H
