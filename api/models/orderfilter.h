#ifndef ORDERFILTER_H
#define ORDERFILTER_H

#include <QString>

namespace api
{
    struct OrderFilter
    {
        bool cancelled;
        bool completed;
        bool inProgress;
        bool notRedeemed;
        bool pending;
        bool redeemed;
        QString query;
    };
}

#endif // ORDERFILTER_H
