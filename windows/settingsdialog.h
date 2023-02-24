#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>

#include "../internals/settingsmanager.h"

namespace Ui {
class SettingsDialog;
}

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget *parent = nullptr);
    ~SettingsDialog();

    void setSettingsManager(SettingsManager *settingsManager);

private slots:
    void on_generalButton_clicked();

    void on_interfaceButton_clicked();

    void on_notificationsButton_clicked();

    void on_installingButton_clicked();

    void on_featuresButton_clicked();

    void on_languageComboBox_currentIndexChanged(int index);

    void on_reportsCheckBox_toggled(bool checked);

    void on_experimentalCheckBox_toggled(bool checked);

    void on_autoStartCheckBox_toggled(bool checked);

    void on_autoLoginCheckBox_toggled(bool checked);

    void on_startingPageComboBox_currentIndexChanged(int index);

    void on_gogStoreRadioButton_toggled(bool checked);

    void on_hideStoreRadioButton_toggled(bool checked);

    void on_messageNotificationPlaceComboBox_currentIndexChanged(int index);

    void on_messageNotificationKindComboBox_currentIndexChanged(int index);

    void on_friendInvitePlaceComboBox_currentIndexChanged(int index);

    void on_friendInviteKindComboBox_currentIndexChanged(int index);

    void on_gameInvitePlaceComboBox_currentIndexChanged(int index);

    void on_gameInviteKindComboBox_currentIndexChanged(int index);

    void on_friendOnlinePlaceComboBox_currentIndexChanged(int index);

    void on_friendOnlineKindComboBox_currentIndexChanged(int index);

    void on_friendGamingPlaceComboBox_currentIndexChanged(int index);

    void on_friendGamingKindComboBox_currentIndexChanged(int index);

    void on_installationNotificationPlaceComboBox_currentIndexChanged(int index);

    void on_installationNotificationKindComboBox_currentIndexChanged(int index);

    void on_wishlistNotificationPlaceComboBox_currentIndexChanged(int index);

    void on_wishlistNotificationKindComboBox_currentIndexChanged(int index);

    void on_desktopPositionComboBox_currentIndexChanged(int index);

    void on_overlayPositionComboBox_currentIndexChanged(int index);

    void on_notificationVolumeSlider_valueChanged(int value);

    void on_showFriendsCheckBox_toggled(bool checked);

    void on_preferredLanguageComboBox_currentIndexChanged(int index);

    void on_installationFolderButton_clicked();

    void on_shortcutCheckBox_toggled(bool checked);

    void on_temporaryUpdateButton_clicked();

    void on_downloadsFolderButton_clicked();

    void on_pauseDownloadsCheckBox_toggled(bool checked);

    void on_bandwidthLimitCheckBox_toggled(bool checked);

    void on_bandwidthLimitSpinBox_valueChanged(int arg1);

    void on_bandwidthLimitComboBox_currentIndexChanged(int index);

    void on_scheduleLimitCheckBox_toggled(bool checked);

    void on_scheduleLimitFromTimeEdit_userTimeChanged(const QTime &time);

    void on_scheduleLimitToTimeEdit_userTimeChanged(const QTime &time);

    void on_autoUpdateCheckbox_toggled(bool checked);

    void on_cloudSavesCheckbox_toggled(bool checked);

    void on_achievementsCheckbox_toggled(bool checked);

    void on_timeTrackingCheckbox_toggled(bool checked);

    void on_overlayCheckbox_toggled(bool checked);

private:
    SettingsManager *settingsManager = nullptr;

    Ui::SettingsDialog *ui;
    QPushButton *selectedSection;
};

#endif // SETTINGSDIALOG_H
