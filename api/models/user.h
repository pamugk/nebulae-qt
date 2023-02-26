#ifndef USER_H
#define USER_H

#include <QDateTime>
#include <QString>
#include <QVector>
#include <QUrl>

namespace api
{
    struct Avatar
    {
        QString gogImageId;
        QUrl small;
        QUrl small2X;
        QUrl medium;
        QUrl medium2X;
        QUrl large;
        QUrl large2X;
        QUrl sdkImg32;
        QUrl sdkImg64;
        QUrl sdkImg184;
        QUrl menuSmall;
        QUrl menuSmall2;
        QUrl menuBig;
        QUrl menuBig2;
    };

    struct User
    {
        QString id;
        QString username;
        QDateTime created;
        Avatar avatar;
        bool employee;
    };
}

#endif // USER_H
