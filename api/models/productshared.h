#ifndef PRODUCTSHARED_H
#define PRODUCTSHARED_H

#include <QtGlobal>

#include <QDateTime>
#include <QString>

namespace api
{
    struct Availability
    {
        bool available;
        bool availableInAccount;
    };

    struct PlatformAvailability
    {
        bool Windows;
        bool Mac;
        bool Linux;
    };

    struct ZonedDateTime
    {
        QDateTime date;
        quint8 timezoneType;
        QString timezone;
    };
}

#endif // PRODUCTSHARED_H
