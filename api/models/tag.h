#ifndef TAG_H
#define TAG_H

#include <QString>

namespace api
{
    struct Tag
    {
        QString id;
        QString name;
        unsigned int productCount;
    };
}

#endif // TAG_H
