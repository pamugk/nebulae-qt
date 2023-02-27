#ifndef NAVIGATIONDESTINATION_H
#define NAVIGATIONDESTINATION_H

#include <QVariant>

#include "./pages.h"

struct NavigationDestination
{
    Page page;
    QVariant parameters;
};

#endif // NAVIGATIONDESTINATION_H
