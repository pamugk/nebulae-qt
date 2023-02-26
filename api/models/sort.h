#ifndef SORT_H
#define SORT_H

#include <QString>

namespace api
{
    struct SortOrder
    {
        QString field;
        bool ascending;
    };
}

#endif // SORT_H
