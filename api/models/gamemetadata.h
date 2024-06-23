#ifndef GAMEMETADATA_H
#define GAMEMETADATA_H

#include <QString>
#include <QVariantMap>

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
        QVariantMap name;
        QString slug;
    };
}

#endif // GAMEMETADATA_H
