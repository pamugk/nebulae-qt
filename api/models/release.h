#ifndef RELEASE_H
#define RELEASE_H

#include <QDateTime>
#include <QString>
#include <QVector>

#include "game.h"
#include "metatag.h"

namespace api
{
    struct Release
    {
        QString id;
        QString gameId;
        QString platformId;
        QString externalId;
        QVector<QString> dlcsIds;
        QVector<PlatformRelease> dlcs;
        QString parentId;
        QVector<MetaTag> supportedOperatingSystems;
        QVector<QString> availableLanguages;
        QDateTime firstReleaseDate;
        Game game;
        QVariantMap title;
        QVariantMap sortingTitle;
        QString type;
        QVariantMap summary;
        QVector<NamedVideo> videos;
        QVector<IdMetaTag> gameModes;
        QString logo;
    };
}

#endif // RELEASE_H
