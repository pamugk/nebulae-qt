#ifndef SETTINGSMANAGER_H
#define SETTINGSMANAGER_H

#include <QtGlobal>

#include <QObject>
#include <QSettings>
#include <QTime>

#include "./pages.h"

enum Store
{
    GOG,
    HIDDEN
};

enum NotificationPlace
{
    DESKTOP_OVERLAY,
    DESKTOP,
    OVERLAY,
    NONE
};

enum NotificationKind
{
    SOUND,
    MUTED
};

enum NotificationPosition
{
    TOP_LEFT,
    TOP_RIGHT,
    BOTTOM_RIGHT,
    BOTTOM_LEFT
};

enum DataTransferSpeedUnit
{
    KBPS,
    MBPS
};

class SettingsManager : public QObject
{
    Q_OBJECT
public:
    explicit SettingsManager(QObject *parent = nullptr);

    // General settings
    const QString getLanguage();
    void setLanguage(const QString &value);

    bool isReportBugs();
    void setReportBugs(bool value);

    bool isExperimental();
    void setExperimental(bool value);

    bool isAutoStart();
    void setAutoStart(bool value);

    bool isAutoLogin();
    void setAutoLogin(bool value);

    // Interface settings
    Page getStartingPage();
    void setStartingPage(Page value);

    Store getPreferredStore();
    void setPreferredStore(Store value);

    bool isShowFriendsSidebar();
    void setShowFriendsSidebar(bool value);

    // Notifications settings
    NotificationPlace getMessageNotificationPlace();
    void setMessageNotificationPlace(NotificationPlace value);

    NotificationKind getMessageNotificationKind();
    void setMessageNotificationKind(NotificationKind value);

    NotificationPlace getFriendInviteNotificationPlace();
    void setFriendInviteNotificationPlace(NotificationPlace value);

    NotificationKind getFriendInviteNotificationKind();
    void setFriendInviteNotificationKind(NotificationKind value);

    NotificationPlace getGameInviteNotificationPlace();
    void setGameInviteNotificationPlace(NotificationPlace value);

    NotificationKind getGameInviteNotificationKind();
    void setGameInviteNotificationKind(NotificationKind value);

    NotificationPlace getFriendOnlineNotificationPlace();
    void setFriendOnlineNotificationPlace(NotificationPlace value);

    NotificationKind getFriendOnlineNotificationKind();
    void setFriendOnlineNotificationKind(NotificationKind value);

    NotificationPlace getFriendGameNotificationPlace();
    void setFriendGameNotificationPlace(NotificationPlace value);

    NotificationKind getFriendGameNotificationKind();
    void setFriendGameNotificationKind(NotificationKind value);

    NotificationPlace getInstallationNotificationPlace();
    void setInstallationNotificationPlace(NotificationPlace value);

    NotificationKind getInstallationNotificationKind();
    void setInstallationNotificationKind(NotificationKind value);

    NotificationPlace getWishlistNotificationPlace();
    void setWishlistNotificationPlace(NotificationPlace value);

    NotificationKind getWishlistNotificationKind();
    void setWishlistNotificationKind(NotificationKind value);

    NotificationPosition getNotificationsDesktopPosition();
    void setNotificationsDesktopPosition(NotificationPosition value);

    NotificationPosition getNotificationsOverlayPosition();
    void setNotificationsOverlayPosition(NotificationPosition value);

    quint8 getNotificationsVolume();
    void setNotificationsVolume(quint8 value);

    // Installation & updates settings
    const QString getPreferredGameLanguage();
    void setPreferredGameLanguage(const QString &value);

    const QString getInstallationFolder();
    void setInstallationFolder(const QString &value);

    bool isCreateShortcut();
    void setCreateShortcut(bool value);

    const QString getUpdatesFolder();
    void setUpdatesFolder(const QString &value);

    const QString getDownloadsFolder();
    void setDownloadsFolder(const QString &value);

    bool isPauseDownloads();
    void setPauseDownloads(bool value);

    bool isLimitBandwidth();
    void setLimitBandwidth(bool value);

    quint32 getBandwidthLimit();
    void setBandwidthLimit(quint32 value);

    DataTransferSpeedUnit getBandwidthLimitUnit();
    void setBandwidthLimitUnit(DataTransferSpeedUnit value);

    bool isScheduleLimit();
    void setScheduleLimit(bool value);

    const QTime getScheduleLimitFrom();
    void setScheduleLimitFrom(const QTime &value);

    const QTime getScheduleLimitTo();
    void setScheduleLimitTo(const QTime &value);

    // Game features settings
    bool isAutoUpdateGames();
    void setAutoUpdateGames(bool value);

    bool isEnableCloudSaves();
    void setEnableCloudSaves(bool value);

    bool isEnableAchievements();
    void setEnableAchievements(bool value);

    bool isTrackingGameTime();
    void setTrackingGameTime(bool value);

    bool isEnableOverlay();
    void setEnableOverlay(bool value);

signals:

private:
    QSettings settings;
};

#endif // SETTINGSMANAGER_H
