#ifndef DATABASE_H
#define DATABASE_H

#include <QVector>

#include "../api/models/platformachievement.h"
#include "../api/models/release.h"
#include "../api/models/userlibraryfilters.h"
#include "../api/models/userlibraryrequest.h"
#include "../api/models/userrelease.h"

namespace db
{
    void initialize();

    QVector<api::Release> getUserReleases(const QString &userId, const api::SearchUserReleasesRequest &data);
    api::UserLibraryFilters getUserReleasesFilters(const QString &userId);
    std::tuple<QString, QString> getUserReleaseToMap(const QString &userId);
    std::tuple<QString, QString> getUserReleaseToUpdateAchievements(const QString &userId);
    void savePlatformReleaseAchievements(const QString &platformId, const QString &platformReleaseId,
                                         const QVector<api::PlatformAchievement> &achievements);
    void saveRelease(const api::Release &release);
    void saveUserReleases(const QString &userId, const QVector<api::UserRelease> &releases);
}

#endif
