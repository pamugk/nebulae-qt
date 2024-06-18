#include "jobmanager.h"

#include <QJsonDocument>
#include <QNetworkReply>

#include "../api/utils/userreleaseserialization.h"

JobManager::JobManager(api::GogApiClient *apiClient, QObject *parent)
    : QObject{parent},
    apiClient(apiClient),
    libraryReply(nullptr)
{

}

JobManager::~JobManager()
{
    if (libraryReply != nullptr)
    {
        libraryReply->abort();
    }
}

void JobManager::setAuthenticated(bool authenticated)
{
    if (libraryReply != nullptr)
    {
        libraryReply->abort();
    }

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
                emit receivedUserLibrary(data);
            }
            else if (networkReply->error() != QNetworkReply::OperationCanceledError)
            {
                qDebug() << "Failed to load user's library: "
                         << networkReply->error() << networkReply->errorString()
                         << QString(networkReply->readAll()).toUtf8();
            }


            networkReply->deleteLater();
        });
    }
}
