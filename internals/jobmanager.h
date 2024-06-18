#ifndef JOBMANAGER_H
#define JOBMANAGER_H

#include <QObject>

#include "../api/gogapiclient.h"
#include "../api/models/userrelease.h"

class JobManager : public QObject
{
    Q_OBJECT
public:
    explicit JobManager(api::GogApiClient *apiClient, QObject *parent = nullptr);
    ~JobManager();

signals:
    void receivedUserLibrary(const api::GetUserReleasesResponse &data);

public slots:
    void setAuthenticated(bool authenticated);

private:
    api::GogApiClient *apiClient;
    QNetworkReply *libraryReply;
};

#endif // JOBMANAGER_H
