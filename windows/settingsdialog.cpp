#include <QFileDialog>

#include "settingsdialog.h"
#include "ui_settingsdialog.h"

SettingsDialog::SettingsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SettingsDialog)
{
    ui->setupUi(this);
    ui->stackedWidget->setCurrentWidget(ui->generalPage);
    selectedSection = ui->generalButton;

    ui->languageComboBox->addItems({"English"});

    ui->preferredLanguageComboBox->addItems({"English"});
    ui->startingPageComboBox->addItems(
    {
        tr("Discover"), tr("Recent"),
        tr("Store"), tr("All games"), tr("Wishlist"), tr("Order history"),
        tr("Owned games"), tr("Installed"),
        tr("Friends")
    });

    const QStringList notificationPlaces = {"Desktop & overlay", "Desktop", "Overlay", "⸺"};
    ui->messageNotificationPlaceComboBox->addItems(notificationPlaces);
    ui->friendInvitePlaceComboBox->addItems(notificationPlaces);
    ui->gameInvitePlaceComboBox->addItems(notificationPlaces);
    ui->friendOnlinePlaceComboBox->addItems(notificationPlaces);
    ui->friendGamingPlaceComboBox->addItems(notificationPlaces);
    ui->installationNotificationPlaceComboBox->addItems(notificationPlaces);
    ui->wishlistNotificationPlaceComboBox->addItems(notificationPlaces);

    const QStringList notificationKinds = {"Sound", "⸺"};
    ui->messageNotificationKindComboBox->addItems(notificationKinds);
    ui->friendInviteKindComboBox->addItems(notificationKinds);
    ui->gameInviteKindComboBox->addItems(notificationKinds);
    ui->friendOnlineKindComboBox->addItems(notificationKinds);
    ui->friendGamingKindComboBox->addItems(notificationKinds);
    ui->installationNotificationKindComboBox->addItems(notificationKinds);
    ui->wishlistNotificationKindComboBox->addItems(notificationKinds);

    const QStringList notificationPositions = { "Top-left", "Top-right", "Bottom-right", "Bottom-left"};
    ui->desktopPositionComboBox->addItems(notificationPositions);
    ui->desktopPositionComboBox->setCurrentIndex(2);
    ui->overlayPositionComboBox->addItems(notificationPositions);
    ui->overlayPositionComboBox->setCurrentIndex(2);
}

SettingsDialog::~SettingsDialog()
{
    delete ui;
}

void SettingsDialog::setSettingsManager(SettingsManager *settingsManager)
{
    // Setting general settings section
    ui->languageComboBox->setCurrentIndex(0);
    ui->reportsCheckBox->setChecked(settingsManager->isReportBugs());
    ui->experimentalCheckBox->setChecked(settingsManager->isExperimental());
    ui->autoStartCheckBox->setChecked(settingsManager->isAutoStart());
    ui->autoLoginCheckBox->setChecked(settingsManager->isAutoLogin());
    ui->startingPageComboBox->setCurrentIndex(settingsManager->getStartingPage());

    // Setting interface settings section
    auto store = settingsManager->getPreferredStore();
    ui->gogStoreRadioButton->setChecked(store == Store::GOG);
    ui->hideStoreRadioButton->setChecked(store == Store::HIDDEN);
    ui->showFriendsCheckBox->setChecked(settingsManager->isShowFriendsSidebar());

    // Setting notifications settings section
    ui->messageNotificationPlaceComboBox->setCurrentIndex(settingsManager->getMessageNotificationPlace());
    ui->messageNotificationKindComboBox->setCurrentIndex(settingsManager->getMessageNotificationKind());
    ui->friendInvitePlaceComboBox->setCurrentIndex(settingsManager->getFriendInviteNotificationPlace());
    ui->friendInviteKindComboBox->setCurrentIndex(settingsManager->getFriendInviteNotificationKind());
    ui->gameInvitePlaceComboBox->setCurrentIndex(settingsManager->getGameInviteNotificationPlace());
    ui->gameInviteKindComboBox->setCurrentIndex(settingsManager->getGameInviteNotificationKind());
    ui->friendOnlinePlaceComboBox->setCurrentIndex(settingsManager->getFriendOnlineNotificationPlace());
    ui->friendOnlineKindComboBox->setCurrentIndex(settingsManager->getFriendOnlineNotificationKind());
    ui->friendGamingPlaceComboBox->setCurrentIndex(settingsManager->getFriendGameNotificationPlace());
    ui->friendGamingKindComboBox->setCurrentIndex(settingsManager->getFriendGameNotificationKind());
    ui->installationNotificationPlaceComboBox->setCurrentIndex(settingsManager->getInstallationNotificationPlace());
    ui->installationNotificationKindComboBox->setCurrentIndex(settingsManager->getInstallationNotificationKind());
    ui->wishlistNotificationPlaceComboBox->setCurrentIndex(settingsManager->getWishlistNotificationPlace());
    ui->wishlistNotificationKindComboBox->setCurrentIndex(settingsManager->getWishlistNotificationKind());
    ui->desktopPositionComboBox->setCurrentIndex(settingsManager->getNotificationsDesktopPosition());
    ui->overlayPositionComboBox->setCurrentIndex(settingsManager->getNotificationsOverlayPosition());
    ui->notificationVolumeSlider->setValue(settingsManager->getNotificationsVolume());

    // Setting installation settings section
    ui->preferredLanguageComboBox->setCurrentIndex(0);
    ui->shortcutCheckBox->setChecked(settingsManager->isCreateShortcut());
    ui->installationFolderLineEdit->setText(settingsManager->getInstallationFolder());
    ui->temporaryUpdateLineEdit->setText(settingsManager->getUpdatesFolder());
    ui->downloadsFolderLineEdit->setText(settingsManager->getDownloadsFolder());
    ui->pauseDownloadsCheckBox->setChecked(settingsManager->isPauseDownloads());
    ui->bandwidthLimitCheckBox->setChecked(settingsManager->isLimitBandwidth());
    ui->bandwidthLimitSpinBox->setValue(settingsManager->getBandwidthLimit());
    ui->bandwidthLimitComboBox->setCurrentIndex(settingsManager->getBandwidthLimitUnit());
    ui->scheduleLimitCheckBox->setChecked(settingsManager->isScheduleLimit());
    ui->scheduleLimitFromTimeEdit->setTime(settingsManager->getScheduleLimitFrom());
    ui->scheduleLimitToTimeEdit->setTime(settingsManager->getScheduleLimitTo());

    // Settings features settings section
    ui->autoUpdateCheckbox->setChecked(settingsManager->isAutoUpdateGames());
    ui->cloudSavesCheckbox->setChecked(settingsManager->isEnableCloudSaves());
    ui->achievementsCheckbox->setChecked(settingsManager->isEnableAchievements());
    ui->timeTrackingCheckbox->setChecked(settingsManager->isTrackingGameTime());
    ui->overlayCheckbox->setChecked(settingsManager->isEnableOverlay());

    this->settingsManager = settingsManager;
}

void SettingsDialog::on_generalButton_clicked()
{
    selectedSection->setChecked(false);
    selectedSection = ui->generalButton;
    ui->stackedWidget->setCurrentWidget(ui->generalPage);
    selectedSection->setChecked(true);
}


void SettingsDialog::on_interfaceButton_clicked()
{
    selectedSection->setChecked(false);
    selectedSection = ui->interfaceButton;
    ui->stackedWidget->setCurrentWidget(ui->interfacePage);
    selectedSection->setChecked(true);
}


void SettingsDialog::on_notificationsButton_clicked()
{
    selectedSection->setChecked(false);
    selectedSection = ui->notificationsButton;
    ui->stackedWidget->setCurrentWidget(ui->notificationsPage);
    selectedSection->setChecked(true);
}


void SettingsDialog::on_installingButton_clicked()
{
    selectedSection->setChecked(false);
    selectedSection = ui->installingButton;
    ui->stackedWidget->setCurrentWidget(ui->installingPage);
    selectedSection->setChecked(true);
}


void SettingsDialog::on_featuresButton_clicked()
{
    selectedSection->setChecked(false);
    selectedSection = ui->featuresButton;
    ui->stackedWidget->setCurrentWidget(ui->featuresPage);
    selectedSection->setChecked(true);
}

void SettingsDialog::on_languageComboBox_currentIndexChanged(int index)
{
    if (settingsManager != nullptr)
    {

    }
}

void SettingsDialog::on_reportsCheckBox_toggled(bool checked)
{
    if (settingsManager != nullptr)
    {
        settingsManager->setReportBugs(checked);
    }
}

void SettingsDialog::on_experimentalCheckBox_toggled(bool checked)
{
    if (settingsManager != nullptr)
    {
        settingsManager->setExperimental(checked);
    }
}

void SettingsDialog::on_autoStartCheckBox_toggled(bool checked)
{
    if (settingsManager != nullptr)
    {
        settingsManager->setAutoStart(checked);
    }
}

void SettingsDialog::on_autoLoginCheckBox_toggled(bool checked)
{
    if (settingsManager != nullptr)
    {
        settingsManager->setAutoLogin(checked);
    }
}

void SettingsDialog::on_startingPageComboBox_currentIndexChanged(int index)
{
    if (settingsManager != nullptr)
    {
        settingsManager->setStartingPage(static_cast<Page>(index));
    }
}

void SettingsDialog::on_gogStoreRadioButton_toggled(bool checked)
{
    if (settingsManager != nullptr)
    {
        ui->hideStoreRadioButton->setChecked(false);
        if (settingsManager != nullptr)
        {
            settingsManager->setPreferredStore(Store::GOG);
        }
    }
}

void SettingsDialog::on_hideStoreRadioButton_toggled(bool checked)
{
    if (checked)
    {
        ui->gogStoreRadioButton->setChecked(false);
        if (settingsManager != nullptr)
        {
            settingsManager->setPreferredStore(Store::HIDDEN);
        }
    }
}

void SettingsDialog::on_messageNotificationPlaceComboBox_currentIndexChanged(int index)
{
    if (settingsManager != nullptr)
    {
        settingsManager->setMessageNotificationPlace(static_cast<NotificationPlace>(index));
    }
}

void SettingsDialog::on_messageNotificationKindComboBox_currentIndexChanged(int index)
{
    if (settingsManager != nullptr)
    {
        settingsManager->setMessageNotificationKind(static_cast<NotificationKind>(index));
    }
}

void SettingsDialog::on_friendInvitePlaceComboBox_currentIndexChanged(int index)
{
    if (settingsManager != nullptr)
    {
        settingsManager->setFriendInviteNotificationPlace(static_cast<NotificationPlace>(index));
    }
}

void SettingsDialog::on_friendInviteKindComboBox_currentIndexChanged(int index)
{
    if (settingsManager != nullptr)
    {
        settingsManager->setFriendInviteNotificationKind(static_cast<NotificationKind>(index));
    }
}

void SettingsDialog::on_gameInvitePlaceComboBox_currentIndexChanged(int index)
{
    if (settingsManager != nullptr)
    {
        settingsManager->setGameInviteNotificationPlace(static_cast<NotificationPlace>(index));
    }
}

void SettingsDialog::on_gameInviteKindComboBox_currentIndexChanged(int index)
{
    if (settingsManager != nullptr)
    {
        settingsManager->setGameInviteNotificationKind(static_cast<NotificationKind>(index));
    }
}

void SettingsDialog::on_friendOnlinePlaceComboBox_currentIndexChanged(int index)
{
    if (settingsManager != nullptr)
    {
        settingsManager->setFriendOnlineNotificationPlace(static_cast<NotificationPlace>(index));
    }
}

void SettingsDialog::on_friendOnlineKindComboBox_currentIndexChanged(int index)
{
    if (settingsManager != nullptr)
    {
        settingsManager->setFriendOnlineNotificationKind(static_cast<NotificationKind>(index));
    }
}

void SettingsDialog::on_friendGamingPlaceComboBox_currentIndexChanged(int index)
{
    if (settingsManager != nullptr)
    {
        settingsManager->setFriendGameNotificationPlace(static_cast<NotificationPlace>(index));
    }
}

void SettingsDialog::on_friendGamingKindComboBox_currentIndexChanged(int index)
{
    if (settingsManager != nullptr)
    {
        settingsManager->setFriendGameNotificationKind(static_cast<NotificationKind>(index));
    }
}

void SettingsDialog::on_installationNotificationPlaceComboBox_currentIndexChanged(int index)
{
    if (settingsManager != nullptr)
    {
        settingsManager->setInstallationNotificationPlace(static_cast<NotificationPlace>(index));
    }
}

void SettingsDialog::on_installationNotificationKindComboBox_currentIndexChanged(int index)
{
    if (settingsManager != nullptr)
    {
        settingsManager->setInstallationNotificationKind(static_cast<NotificationKind>(index));
    }
}

void SettingsDialog::on_wishlistNotificationPlaceComboBox_currentIndexChanged(int index)
{
    if (settingsManager != nullptr)
    {
        settingsManager->setWishlistNotificationPlace(static_cast<NotificationPlace>(index));
    }
}

void SettingsDialog::on_wishlistNotificationKindComboBox_currentIndexChanged(int index)
{
    if (settingsManager != nullptr)
    {
        settingsManager->setWishlistNotificationKind(static_cast<NotificationKind>(index));
    }
}

void SettingsDialog::on_desktopPositionComboBox_currentIndexChanged(int index)
{
    if (settingsManager != nullptr)
    {
        settingsManager->setNotificationsDesktopPosition(static_cast<NotificationPosition>(index));
    }
}

void SettingsDialog::on_overlayPositionComboBox_currentIndexChanged(int index)
{
    if (settingsManager != nullptr)
    {
        settingsManager->setNotificationsOverlayPosition(static_cast<NotificationPosition>(index));
    }
}

void SettingsDialog::on_notificationVolumeSlider_valueChanged(int value)
{
    if (settingsManager != nullptr)
    {
        settingsManager->setNotificationsVolume(value);
    }
}

void SettingsDialog::on_showFriendsCheckBox_toggled(bool checked)
{
    if (settingsManager != nullptr)
    {
        settingsManager->setShowFriendsSidebar(checked);
    }
}

void SettingsDialog::on_preferredLanguageComboBox_currentIndexChanged(int index)
{
    if (settingsManager != nullptr)
    {

    }
}

void SettingsDialog::on_installationFolderButton_clicked()
{
    auto oldFolder = settingsManager == nullptr ? QString() : settingsManager->getInstallationFolder();
    auto newInstallationFolder = QFileDialog::getExistingDirectory(this, "Choose installation folder", oldFolder);
    if (!newInstallationFolder.isNull() && !newInstallationFolder.isEmpty())
    {
        if (settingsManager != nullptr)
        {
            settingsManager->setInstallationFolder(newInstallationFolder);
        }
        ui->installationFolderLineEdit->setText(newInstallationFolder);
    }
}

void SettingsDialog::on_shortcutCheckBox_toggled(bool checked)
{
    if (settingsManager != nullptr)
    {
        settingsManager->setCreateShortcut(checked);
    }
}

void SettingsDialog::on_temporaryUpdateButton_clicked()
{
    auto oldFolder = settingsManager == nullptr ? QString() : settingsManager->getUpdatesFolder();
    auto newUpdatesFolder = QFileDialog::getExistingDirectory(this, "Choose updates folder", oldFolder);
    if (!newUpdatesFolder.isNull() && !newUpdatesFolder.isEmpty())
    {
        if (settingsManager != nullptr)
        {
            settingsManager->setUpdatesFolder(newUpdatesFolder);
        }
        ui->temporaryUpdateLineEdit->setText(newUpdatesFolder);
    }
}

void SettingsDialog::on_downloadsFolderButton_clicked()
{
    auto oldFolder = settingsManager == nullptr ? QString() : settingsManager->getDownloadsFolder();
    auto newDownloadsFolder = QFileDialog::getExistingDirectory(this, "Choose downloads folder", oldFolder);
    if (!newDownloadsFolder.isNull() && !newDownloadsFolder.isEmpty())
    {
        if (settingsManager != nullptr)
        {
            settingsManager->setDownloadsFolder(newDownloadsFolder);
        }
        ui->downloadsFolderLineEdit->setText(newDownloadsFolder);
    }
}

void SettingsDialog::on_pauseDownloadsCheckBox_toggled(bool checked)
{
    if (settingsManager != nullptr)
    {
        settingsManager->setPauseDownloads(checked);
    }
}

void SettingsDialog::on_bandwidthLimitCheckBox_toggled(bool checked)
{
    ui->bandwidthLimitToLabel->setEnabled(checked);
    ui->bandwidthLimitSpinBox->setEnabled(checked);
    ui->bandwidthLimitComboBox->setEnabled(checked);
    if (settingsManager != nullptr)
    {
        settingsManager->setLimitBandwidth(checked);
    }
}

void SettingsDialog::on_bandwidthLimitSpinBox_valueChanged(int value)
{
    if (settingsManager != nullptr)
    {
        settingsManager->setBandwidthLimit(value);
    }
}

void SettingsDialog::on_bandwidthLimitComboBox_currentIndexChanged(int index)
{
    if (settingsManager != nullptr)
    {
        settingsManager->setBandwidthLimitUnit(static_cast<DataTransferSpeedUnit>(index));
    }
}

void SettingsDialog::on_scheduleLimitCheckBox_toggled(bool checked)
{
    ui->scheduleLimitFromLabel->setEnabled(checked);
    ui->scheduleLimitFromTimeEdit->setEnabled(checked);
    ui->scheduleLimitToLabel->setEnabled(checked);
    ui->scheduleLimitToTimeEdit->setEnabled(checked);
    if (settingsManager != nullptr)
    {
        settingsManager->setScheduleLimit(checked);
    }
}

void SettingsDialog::on_scheduleLimitFromTimeEdit_userTimeChanged(const QTime &time)
{
    if (settingsManager != nullptr)
    {
        settingsManager->setScheduleLimitFrom(time);
    }
}

void SettingsDialog::on_scheduleLimitToTimeEdit_userTimeChanged(const QTime &time)
{
    if (settingsManager != nullptr)
    {
        settingsManager->setScheduleLimitTo(time);
    }
}

void SettingsDialog::on_autoUpdateCheckbox_toggled(bool checked)
{
    if (settingsManager != nullptr)
    {
        settingsManager->setAutoUpdateGames(checked);
    }
}

void SettingsDialog::on_cloudSavesCheckbox_toggled(bool checked)
{
    if (settingsManager != nullptr)
    {
        settingsManager->setEnableCloudSaves(checked);
    }
}

void SettingsDialog::on_achievementsCheckbox_toggled(bool checked)
{
    if (settingsManager != nullptr)
    {
        settingsManager->setEnableAchievements(checked);
    }
}

void SettingsDialog::on_timeTrackingCheckbox_toggled(bool checked)
{
    if (settingsManager != nullptr)
    {
        settingsManager->setTrackingGameTime(checked);
    }
}

void SettingsDialog::on_overlayCheckbox_toggled(bool checked)
{
    if (settingsManager != nullptr)
    {
        settingsManager->setEnableOverlay(checked);
    }
}

