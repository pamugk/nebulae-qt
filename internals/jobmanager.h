#ifndef JOBMANAGER_H
#define JOBMANAGER_H

#include <QObject>

#include "../api/gogapiclient.h"

class JobManager : public QObject
{
    Q_OBJECT
public:
    explicit JobManager(api::GogApiClient *apiClient, QObject *parent = nullptr);
    ~JobManager();

signals:

public slots:
    void setAuthenticated(bool authenticated, const QString &userId);

private:
    api::GogApiClient *apiClient;
    QNetworkReply *libraryReply;
    QString userId;
};

#endif // JOBMANAGER_H
