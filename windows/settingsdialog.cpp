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

