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
    };

    struct Settings
    {
        QString allowToBeInvitedBy;
        bool allowToBeSearched;
        bool useTwoStepAuthentication;
        bool allowDrmProducts;
    };

    struct User
    {
        QString id;
        QString username;
        QDateTime created;
        Avatar avatar;
        bool employee;
    };

    struct UserFullData : User
    {
        QVector<QString> tags;
        QString email;
        QVector<QString> roles;
        int status;
        QString checksum;
        Settings settings;
        bool passwordSet;
        QVariant marketingConsent;
    };
}

#endif // USER_H
