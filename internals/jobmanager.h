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
    void setAuthenticated(bool authenticated);

protected:
    virtual void timerEvent(QTimerEvent *event) override;

private:
    api::GogApiClient *apiClient;
    QNetworkReply *libraryReleaseReply;
    QNetworkReply *libraryReleaseAchievementsReply;
    QNetworkReply *libraryReleaseUserAchievementsReply;
    QMap<QString, QNetworkReply *> userAchievementsByPlatformsReplies;
    QNetworkReply *userGameplayReply;
    QNetworkReply *userLibraryReply;

    std::optional<int> achievementsTimerId;
    std::optional<int> libraryTimerId;

    void getUserPlatformAchievements(const QString &platform, const QString &pageToken = "");
};

#endif // JOBMANAGER_H
