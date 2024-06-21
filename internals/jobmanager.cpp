#include "jobmanager.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkReply>
#include <QTimerEvent>

#include "../api/utils/releaseserialization.h"
#include "../api/utils/userreleaseserialization.h"
#include "../db/database.h"

JobManager::JobManager(api::GogApiClient *apiClient, QObject *parent)
    : QObject{parent},
    achievementsTimerId(),
    apiClient(apiClient),
    libraryReply(nullptr),
    libraryReleaseReply(nullptr),
    libraryReleaseAchievementsReply(nullptr),
    libraryReleaseUserAchievementsReply(nullptr),
    libraryTimerId()
{

}

JobManager::~JobManager()
{
    if (libraryReply != nullptr)
    {
        libraryReply->abort();
    }
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
}

void JobManager::setAuthenticated(bool authenticated, const QString &userId)
{
    if (libraryReply != nullptr)
    {
        libraryReply->abort();
    }
    this->userId = userId;

    if (authenticated)
    {
        libraryReply = apiClient->getCurrentUserReleases();
        connect(libraryReply, &QNetworkReply::finished,
                this, [this]()
        {
            auto networkReply = libraryReply;
            libraryReply = nullptr;

            if (networkReply->error() == QNetworkReply::NoError)
            {
                auto resultJson = QJsonDocument::fromJson(QString(networkReply->readAll()).toUtf8()).object();
                api::GetUserReleasesResponse data;
                parseGetUserReleasesResponse(resultJson, data);
                db::saveUserReleases(this->userId, data.items);
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
            libraryTimerId = startTimer(std::chrono::minutes(1));
        }
        if (!achievementsTimerId.has_value())
        {
            achievementsTimerId = startTimer(std::chrono::minutes(1));
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
                        db::saveRelease(data);
                    }
                    else if (networkReply->error() != QNetworkReply::OperationCanceledError)
                    {
                        const auto &[platformId, platformReleaseId] = ids;
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
                libraryReleaseAchievementsReply = apiClient->getPlatformReleaseAchievements(platformId, platformReleaseId);
                connect(libraryReleaseAchievementsReply, &QNetworkReply::finished,
                        this, [this, ids = std::make_tuple(platformId, platformReleaseId)]()
                {
                    auto networkReply = libraryReleaseAchievementsReply;
                    libraryReleaseAchievementsReply = nullptr;
                    if (networkReply->error() == QNetworkReply::NoError)
                    {
                        const auto &[platformId, platformReleaseId] = ids;
                        qDebug() << "Received achievements" << platformId << platformReleaseId;
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
