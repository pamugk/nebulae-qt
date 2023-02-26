#ifndef METATAG_H
#define METATAG_H

#include <QtGlobal>

#include <QString>

namespace api
{
    struct MetaTag
    {
        QString name;
        QString slug;
    };

    struct HierarchicalMetaTag: public MetaTag
    {
        quint8 level;
    };
}

#endif // METATAG_H
