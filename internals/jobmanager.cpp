#include "jobmanager.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkReply>
#include <QTimerEvent>

#include "../api/utils/platformachievementserialization.h"
#include "../api/utils/releaseserialization.h"
#include "../api/utils/statisticsserialization.h"
#include "../api/utils/userreleaseserialization.h"
#include "../db/database.h"

JobManager::JobManager(api::GogApiClient *apiClient, QObject *parent)
    : QObject{parent},
    achievementsTimerId(),
    apiClient(apiClient),
    libraryReleaseReply(nullptr),
    libraryReleaseAchievementsReply(nullptr),
    libraryReleaseUserAchievementsReply(nullptr),
    libraryTimerId(),
    userGameplayReply(nullptr),
    userLibraryReply(nullptr)
{

}

JobManager::~JobManager()
{
    if (libraryReleaseReply != nullptr)
    {
        libraryReleaseReply->abort();
    }
    if (libraryReleaseAchievementsReply != nullptr)
    {
        libraryReleaseAchievementsReply->abort();
    }
    if (libraryReleaseUserAchievementsReply != nullptr)
    {
        libraryReleaseUserAchievementsReply->abort();
    }
    for (auto item : std::as_const(userAchievementsByPlatformsReplies))
    {
        if (item != nullptr)
        {
            item->abort();
        }
    }
    userAchievementsByPlatformsReplies.clear();
    if (userGameplayReply != nullptr)
    {
        userGameplayReply->abort();
    }
    if (userLibraryReply != nullptr)
    {
        userLibraryReply->abort();
    }
}

void JobManager::getUserPlatformAchievements(const QString &platform, const QString &pageToken)
{
    userAchievementsByPlatformsReplies[platform] = apiClient->getCurrentUserPlatformAchievements(platform, pageToken);
    connect(userAchievementsByPlatformsReplies[platform], &QNetworkReply::finished, this, [this, platform, pageToken]()
    {
       auto networkReply = userAchievementsByPlatformsReplies[platform];
       userAchievementsByPlatformsReplies[platform] = nullptr;
       if (networkReply->error() == QNetworkReply::NoError)
       {
           auto resultJson = QJsonDocument::fromJson(QString(networkReply->readAll()).toUtf8()).object();
           api::GetUserPlatformAchievementsResponse data;
           parseGetUserPlatformAchievementsResponse(resultJson, data);
           qDebug() << "Received user achievements on platform " << platform;
           db::saveUserPlatformAchievements(apiClient->currentUserId(), data.items);
           if (!data.nextPageToken.isNull())
           {
               getUserPlatformAchievements(platform, data.nextPageToken);
           }
       }
       else if (networkReply->error() != QNetworkReply::OperationCanceledError)
       {
           qDebug() << "Failed to get user's achievements on a platform, retrying: "
                    << networkReply->error() << networkReply->errorString()
                    << QString(networkReply->readAll()).toUtf8();
           getUserPlatformAchievements(platform, pageToken);
       }

       networkReply->deleteLater();
    });
}

void JobManager::setAuthenticated(bool authenticated)
{
    if (!userAchievementsByPlatformsReplies.empty())
    {
        for (auto item : std::as_const(userAchievementsByPlatformsReplies))
        {
            if (item != nullptr)
            {
                item->abort();
            }
        }
        userAchievementsByPlatformsReplies.clear();
    }
    if (userGameplayReply != nullptr)
    {
        userGameplayReply->abort();
    }
    if (userLibraryReply != nullptr)
    {
        userLibraryReply->abort();
    }

    if (authenticated)
    {
        userGameplayReply = apiClient->getCurrentUserGameTimeStatistics();
        connect(userGameplayReply, &QNetworkReply::finished,
                this, [this]()
        {
            auto networkReply = userGameplayReply;
            userGameplayReply = nullptr;

            if (networkReply->error() == QNetworkReply::NoError)
            {
                auto resultJson = QJsonDocument::fromJson(QString(networkReply->readAll()).toUtf8()).object();
                api::GetUserGameTimeStatisticsResponse data;
                parseGetUserGameTimeStatisticsResponse(resultJson, data);
                db::saveUserGameTimeStatistics(this->apiClient->currentUserId(), data.items);
            }
            else if (networkReply->error() != QNetworkReply::OperationCanceledError)
            {
                qDebug() << "Failed to load user's game time statistics: "
                         << networkReply->error() << networkReply->errorString()
                         << QString(networkReply->readAll()).toUtf8();
            }


            networkReply->deleteLater();
        });
        userLibraryReply = apiClient->getCurrentUserReleases();
        connect(userLibraryReply, &QNetworkReply::finished,
                this, [this]()
        {
            auto networkReply = userLibraryReply;
            userLibraryReply = nullptr;

            if (networkReply->error() == QNetworkReply::NoError)
            {
                auto resultJson = QJsonDocument::fromJson(QString(networkReply->readAll()).toUtf8()).object();
                api::GetUserReleasesResponse data;
                parseGetUserReleasesResponse(resultJson, data);
                db::saveUserReleases(this->apiClient->currentUserId(), data.items);

                QSet<QString> platforms;
                for (const api::UserRelease &userRelease : std::as_const(data.items))
                {
                    platforms << userRelease.platformId;
                }

                for (const QString &platform: std::as_const(platforms))
                {
                    if (!platform.isNull())
                    {
                        getUserPlatformAchievements(platform);
                    }
                }
            }
            else if (networkReply->error() != QNetworkReply::OperationCanceledError)
            {
                qDebug() << "Failed to load user's library: "
                         << networkReply->error() << networkReply->errorString()
                         << QString(networkReply->readAll()).toUtf8();
            }


            networkReply->deleteLater();
        });

        if (!libraryTimerId.has_value())
        {
            libraryTimerId = startTimer(std::chrono::seconds(30));
        }
        if (!achievementsTimerId.has_value())
        {
            achievementsTimerId = startTimer(std::chrono::seconds(30));
        }
    }
    else
    {
        if (libraryTimerId.has_value())
        {
            killTimer(libraryTimerId.value());
        }
        if (achievementsTimerId.has_value())
        {
            killTimer(achievementsTimerId.value());
        }
    }
}

void JobManager::timerEvent(QTimerEvent *event)
{
    const QString userId = this->apiClient->currentUserId();
    if (userId.isNull())
    {
        return;
    }

    // Fetching info relevant to current user
    if (event->timerId() == libraryTimerId)
    {
        if (libraryReleaseReply == nullptr)
        {
            const auto &[platformId, platformReleaseId] = db::getUserReleaseToMap(userId);
            if (!platformReleaseId.isNull())
            {
                libraryReleaseReply = apiClient->getPlatformRelease(platformId, platformReleaseId);
                connect(libraryReleaseReply, &QNetworkReply::finished,
                        this, [this, ids = std::make_tuple(platformId, platformReleaseId)]()
                {
                    auto networkReply = libraryReleaseReply;
                    libraryReleaseReply = nullptr;
                    if (networkReply->error() == QNetworkReply::NoError)
                    {
                        auto resultJson = QJsonDocument::fromJson(QString(networkReply->readAll()).toUtf8()).object();
                        api::Release data;
                        parseRelease(resultJson, data);
                        qDebug() << "Saving release data" << data.id;
                        db::saveRelease(data);
                    }
                    else if (networkReply->error() == QNetworkReply::ContentNotFoundError)
                    {
                        const auto &[platformId, platformReleaseId] = ids;
                        api::Release stubData;
                        stubData.platformId = platformId;
                        stubData.externalId = platformReleaseId;
                        qDebug() << "Found an unknown product, saving stub mapping: " << platformId << platformReleaseId;
                        db::saveRelease(stubData);
                    }
                    else if (networkReply->error() != QNetworkReply::OperationCanceledError)
                    {
                        qDebug() << "Failed to fetch user's release info: "
                                 << networkReply->error() << networkReply->errorString()
                                 << QString(networkReply->readAll()).toUtf8();
                    }

                    networkReply->deleteLater();
                });
            }
        }
    }
    else if (event->timerId() == achievementsTimerId)
    {
        if (libraryReleaseAchievementsReply == nullptr)
        {
            const auto &[platformId, platformReleaseId] = db::getUserReleaseToUpdateAchievements(userId);
            if (!platformReleaseId.isNull())
            {
                libraryReleaseAchievementsReply = apiClient->getPlatformReleaseAchievements(platformId, platformReleaseId,
                                                                                            "en-US");
                connect(libraryReleaseAchievementsReply, &QNetworkReply::finished,
                        this, [this, ids = std::make_tuple(platformId, platformReleaseId)]()
                {
                    auto networkReply = libraryReleaseAchievementsReply;
                    libraryReleaseAchievementsReply = nullptr;
                    if (networkReply->error() == QNetworkReply::NoError)
                    {
                        auto resultJson = QJsonDocument::fromJson(QString(networkReply->readAll()).toUtf8()).object();
                        api::GetPlatformReleaseAchievementsResponse data;
                        parseGetPlatformReleaseAchievementsResponse(resultJson, data);
                        const auto &[platformId, platformReleaseId] = ids;
                        // TODO: handle pagination for games with lots of achievements... somehow.
                        db::savePlatformReleaseAchievements(platformId, platformReleaseId, data.items);
                        qDebug() << "Received achievements for" << platformId << platformReleaseId;
                    }
                    else if (networkReply->error() != QNetworkReply::OperationCanceledError)
                    {
                        qDebug() << "Failed to fetch user's release achievements: "
                                 << networkReply->error() << networkReply->errorString()
                                 << QString(networkReply->readAll()).toUtf8();
                    }

                    networkReply->deleteLater();
                });
            }
        }
    }
}
