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

protected:
    virtual void timerEvent(QTimerEvent *event) override;

private:
    api::GogApiClient *apiClient;
    QNetworkReply *libraryReply;
    QNetworkReply *libraryReleaseReply;
    QNetworkReply *libraryReleaseAchievementsReply;
    QNetworkReply *libraryReleaseUserAchievementsReply;
    QString userId;

    std::optional<int> achievementsTimerId;
    std::optional<int> libraryTimerId;
};

#endif // JOBMANAGER_H
