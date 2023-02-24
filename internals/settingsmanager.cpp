#include <QStandardPaths>

#include "settingsmanager.h"

SettingsManager::SettingsManager(QObject *parent)
    : QObject{parent}
{
}

const QString SettingsManager::getLanguage()
{
    return settings.value("/general/language", "en_US").toString();
}

void SettingsManager::setLanguage(const QString &value)
{
    settings.setValue("/general/language", value);
}

bool SettingsManager::isReportBugs()
{
    return settings.value("/general/report_bugs", false).toBool();
}

void SettingsManager::setReportBugs(bool value)
{
    settings.setValue("/general/report_bugs", value);
}

bool SettingsManager::isExperimental()
{
    return settings.value("/general/experimental", false).toBool();
}

void SettingsManager::setExperimental(bool value)
{
    settings.setValue("/general/experimental", value);
}

bool SettingsManager::isAutoStart()
{
    return settings.value("/general/auto_start", false).toBool();
}

void SettingsManager::setAutoStart(bool value)
{
    settings.setValue("/general/auto_start", value);
}

bool SettingsManager::isAutoLogin()
{
    return settings.value("/general/auto_login", true).toBool();
}

void SettingsManager::setAutoLogin(bool value)
{
    settings.setValue("/general/auto_login", value);
}

Page SettingsManager::getStartingPage()
{
    return static_cast<Page>(settings.value("/general/starting_page", Page::DISCOVER).toInt());
}

void SettingsManager::setStartingPage(Page value)
{
    settings.setValue("/general/starting_page", value);
}

Store SettingsManager::getPreferredStore()
{
    return static_cast<Store>(settings.value("/interface/preferred_store", Store::GOG).toInt());
}

void SettingsManager::setPreferredStore(Store value)
{
    settings.setValue("/interface/starting_page", value);
}

bool SettingsManager::isShowFriendsSidebar()
{
    return settings.value("/interface/show_friends_sidebar", false).toBool();
}

void SettingsManager::setShowFriendsSidebar(bool value)
{
    settings.setValue("/interface/show_friends_sidebar", value);
}

NotificationPlace SettingsManager::getMessageNotificationPlace()
{
    return static_cast<NotificationPlace>(settings.value("/notifications/message/place", NotificationPlace::DESKTOP_OVERLAY).toInt());
}

void SettingsManager::setMessageNotificationPlace(NotificationPlace value)
{
    settings.setValue("/notifications/message/place", value);
}

NotificationKind SettingsManager::getMessageNotificationKind()
{
    return static_cast<NotificationKind>(settings.value("/notifications/message/kind", NotificationKind::SOUND).toInt());
}

void SettingsManager::setMessageNotificationKind(NotificationKind value)
{
    settings.setValue("/notifications/message/kind", value);
}

NotificationPlace SettingsManager::getFriendInviteNotificationPlace()
{
    return static_cast<NotificationPlace>(settings.value("/notifications/friend_invite/place", NotificationPlace::DESKTOP_OVERLAY).toInt());
}

void SettingsManager::setFriendInviteNotificationPlace(NotificationPlace value)
{
    settings.setValue("/notifications/friend_invite/place", value);
}

NotificationKind SettingsManager::getFriendInviteNotificationKind()
{
    return static_cast<NotificationKind>(settings.value("/notifications/friend_invite/kind", NotificationKind::SOUND).toInt());
}

void SettingsManager::setFriendInviteNotificationKind(NotificationKind value)
{
    settings.setValue("/notifications/friend_invite/kind", value);
}

NotificationPlace SettingsManager::getGameInviteNotificationPlace()
{
    return static_cast<NotificationPlace>(settings.value("/notifications/game_invite/place", NotificationPlace::DESKTOP_OVERLAY).toInt());
}

void SettingsManager::setGameInviteNotificationPlace(NotificationPlace value)
{
    settings.setValue("/notifications/game_invite/place", value);
}

NotificationKind SettingsManager::getGameInviteNotificationKind()
{
    return static_cast<NotificationKind>(settings.value("/notifications/game_invite/kind", NotificationKind::SOUND).toInt());
}

void SettingsManager::setGameInviteNotificationKind(NotificationKind value)
{
    settings.setValue("/notifications/game_invite/kind", value);
}

NotificationPlace SettingsManager::getFriendOnlineNotificationPlace()
{
    return static_cast<NotificationPlace>(settings.value("/notifications/friend_online/place", NotificationPlace::DESKTOP).toInt());
}

void SettingsManager::setFriendOnlineNotificationPlace(NotificationPlace value)
{
    settings.setValue("/notifications/friend_online/place", value);
}

NotificationKind SettingsManager::getFriendOnlineNotificationKind()
{
    return static_cast<NotificationKind>(settings.value("/notifications/friend_online/kind", NotificationKind::MUTED).toInt());
}

void SettingsManager::setFriendOnlineNotificationKind(NotificationKind value)
{
    settings.setValue("/notifications/friend_online/kind", value);
}

NotificationPlace SettingsManager::getFriendGameNotificationPlace()
{
    return static_cast<NotificationPlace>(settings.value("/notifications/friend_game/place", NotificationPlace::DESKTOP).toInt());
}

void SettingsManager::setFriendGameNotificationPlace(NotificationPlace value)
{
    settings.setValue("/notifications/friend_game/place", value);
}

NotificationKind SettingsManager::getFriendGameNotificationKind()
{
    return static_cast<NotificationKind>(settings.value("/notifications/friend_game/kind", NotificationKind::MUTED).toInt());
}

void SettingsManager::setFriendGameNotificationKind(NotificationKind value)
{
    settings.setValue("/notifications/friend_game/kind", value);
}

NotificationPlace SettingsManager::getInstallationNotificationPlace()
{
    return static_cast<NotificationPlace>(settings.value("/notifications/installation/place", NotificationPlace::DESKTOP_OVERLAY).toInt());
}

void SettingsManager::setInstallationNotificationPlace(NotificationPlace value)
{
    settings.setValue("/notifications/installation/place", value);
}

NotificationKind SettingsManager::getInstallationNotificationKind()
{
    return static_cast<NotificationKind>(settings.value("/notifications/installation/kind", NotificationKind::MUTED).toInt());
}

void SettingsManager::setInstallationNotificationKind(NotificationKind value)
{
    settings.setValue("/notifications/installation/kind", value);
}

NotificationPlace SettingsManager::getWishlistNotificationPlace()
{
    return static_cast<NotificationPlace>(settings.value("/notifications/wishlist/place", NotificationPlace::DESKTOP_OVERLAY).toInt());
}

void SettingsManager::setWishlistNotificationPlace(NotificationPlace value)
{
    settings.setValue("/notifications/wishlist/place", value);
}

NotificationKind SettingsManager::getWishlistNotificationKind()
{
    return static_cast<NotificationKind>(settings.value("/notifications/wishlist/kind", NotificationKind::MUTED).toInt());
}

void SettingsManager::setWishlistNotificationKind(NotificationKind value)
{
    settings.setValue("/notifications/wishlist/kind", value);
}

NotificationPosition SettingsManager::getNotificationsDesktopPosition()
{
    return static_cast<NotificationPosition>(settings.value("/notifications/desktop_position", NotificationPosition::BOTTOM_RIGHT).toInt());
}

void SettingsManager::setNotificationsDesktopPosition(NotificationPosition value)
{
    settings.setValue("/notifications/desktop_position", value);
}

NotificationPosition SettingsManager::getNotificationsOverlayPosition()
{
    return static_cast<NotificationPosition>(settings.value("/notifications/overlay_position", NotificationPosition::BOTTOM_RIGHT).toInt());
}

void SettingsManager::setNotificationsOverlayPosition(NotificationPosition value)
{
    settings.setValue("/notifications/overlay_position", value);
}

quint8 SettingsManager::getNotificationsVolume()
{
    return settings.value("/notifications/volume", 100).toUInt();
}

void SettingsManager::setNotificationsVolume(quint8 value)
{
    settings.setValue("/notifications/volume", value);
}

const QString SettingsManager::getPreferredGameLanguage()
{
    return settings.value("/installing/game_language", "en_US").toString();
}

void SettingsManager::setPreferredGameLanguage(const QString &value)
{
    settings.setValue("/installing/game_language", value);
}

const QString SettingsManager::getInstallationFolder()
{
    auto defaultFolder = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
    return settings.value("/installing/installation_folder", defaultFolder).toString();
}

void SettingsManager::setInstallationFolder(const QString &value)
{
    settings.setValue("/installing/installation_folder", value);
}

bool SettingsManager::isCreateShortcut()
{
    return settings.value("/installing/create_shortcut", true).toBool();
}

void SettingsManager::setCreateShortcut(bool value)
{
    settings.setValue("/installing/create_shortcut", value);
}

const QString SettingsManager::getUpdatesFolder()
{
    auto defaultFolder = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
    return settings.value("/installing/updates_folder", defaultFolder).toString();
}

void SettingsManager::setUpdatesFolder(const QString &value)
{
    settings.setValue("/installing/updates_folder", value);
}

const QString SettingsManager::getDownloadsFolder()
{
    auto defaultFolder = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);
    return settings.value("/installing/downloads_folder", defaultFolder).toString();
}

void SettingsManager::setDownloadsFolder(const QString &value)
{
    settings.setValue("/installing/downloads_folder", value);
}

bool SettingsManager::isPauseDownloads()
{
    return settings.value("/installing/pause_downloads", false).toBool();
}

void SettingsManager::setPauseDownloads(bool value)
{
    settings.setValue("/installing/pause_downloads", value);
}

bool SettingsManager::isLimitBandwidth()
{
    return settings.value("/installing/pause_downloads", false).toBool();
}

void SettingsManager::setLimitBandwidth(bool value)
{
    settings.setValue("/installing/pause_downloads", value);
}

quint32 SettingsManager::getBandwidthLimit()
{
    return settings.value("/installing/bandwidth_limit/value", 1).toUInt();
}

void SettingsManager::setBandwidthLimit(quint32 value)
{
    settings.setValue("/installing/bandwidth_limit/value", value);
}

DataTransferSpeedUnit SettingsManager::getBandwidthLimitUnit()
{
    return static_cast<DataTransferSpeedUnit>(settings.value("/installing/bandwidth_limit/unit", DataTransferSpeedUnit::MBPS).toInt());
}

void SettingsManager::setBandwidthLimitUnit(DataTransferSpeedUnit value)
{
    settings.setValue("/installing/bandwidth_limit/unit", value);
}

bool SettingsManager::isScheduleLimit()
{
    return settings.value("/installing/schedule_limit/enabled", false).toBool();
}

void SettingsManager::setScheduleLimit(bool value)
{
    settings.setValue("/installing/schedule_limit/enabled", value);
}

const QTime SettingsManager::getScheduleLimitFrom()
{
    return settings.value("/installing/schedule_limit/from", QTime(10, 0)).toTime();
}

void SettingsManager::setScheduleLimitFrom(const QTime &value)
{
    settings.setValue("/installing/schedule_limit/from", value);
}

const QTime SettingsManager::getScheduleLimitTo()
{
    return settings.value("/installing/schedule_limit/to", QTime(19, 0)).toTime();
}

void SettingsManager::setScheduleLimitTo(const QTime &value)
{
    settings.setValue("/installing/schedule_limit/to", value);
}

bool SettingsManager::isAutoUpdateGames()
{
    return settings.value("/features/auto_update", true).toBool();
}

void SettingsManager::setAutoUpdateGames(bool value)
{
    settings.setValue("/features/auto_update", value);
}

bool SettingsManager::isEnableCloudSaves()
{
    return settings.value("/features/cloud_saves", true).toBool();
}

void SettingsManager::setEnableCloudSaves(bool value)
{
    settings.setValue("/features/cloud_saves", value);
}

bool SettingsManager::isEnableAchievements()
{
    return settings.value("/features/achievements", true).toBool();
}

void SettingsManager::setEnableAchievements(bool value)
{
    settings.setValue("/features/achievements", value);
}

bool SettingsManager::isTrackingGameTime()
{
    return settings.value("/features/game_time_tracking", true).toBool();
}

void SettingsManager::setTrackingGameTime(bool value)
{
    settings.setValue("/features/game_time_tracking", value);
}

bool SettingsManager::isEnableOverlay()
{
    return settings.value("/features/overlay", true).toBool();
}

void SettingsManager::setEnableOverlay(bool value)
{
    settings.setValue("/features/overlay", value);
}
