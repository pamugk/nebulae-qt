#ifndef METATAG_H
#define METATAG_H

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
        unsigned char level;
    };
}

#endif // METATAG_H
