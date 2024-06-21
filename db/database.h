#ifndef DATABASE_H
#define DATABASE_H

#include <QVector>

#include "../api/models/release.h"
#include "../api/models/userrelease.h"

namespace db
{
    void initialize();

    std::tuple<QString, QString> getUserReleaseToMap(const QString &userId);
    std::tuple<QString, QString> getUserReleaseToUpdateAchievements(const QString &userId);
    void saveRelease(const api::Release &release);
    void saveUserReleases(const QString &userId, const QVector<api::UserRelease> &releases);
}

#endif
