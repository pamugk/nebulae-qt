#ifndef NAVIGATIONDESTINATION_H
#define NAVIGATIONDESTINATION_H

#include <QVariant>

#include "./pages.h"

struct NavigationDestination
{
    Page page;
    QVariant parameters;

    const bool operator==(const NavigationDestination &other)
    {
        return page == other.page && parameters == other.parameters;
    }

    const bool operator!=(const NavigationDestination &other)
    {
        return page != other.page || parameters != other.parameters;
    }
};

#endif // NAVIGATIONDESTINATION_H
