#include "achievementlistitem.h"
#include "ui_achievementlistitem.h"

#include <QNetworkReply>

AchievementListItem::AchievementListItem(const api::PlatformAchievement &data,
                                         const QDate &unlockedDate,
                                         api::GogApiClient *apiClient,
                                         QWidget *parent) :
    QWidget(parent),
    ui(new Ui::AchievementListItem)
{
    ui->setupUi(this);

    QLocale systemLocale = QLocale::system();
    ui->titleLabel->setText(data.name);
    ui->descriptionLabel->setText(data.description);
    QString imageUrl;
    if (unlockedDate.isNull())
    {
        imageUrl = data.imageLockedUrl;
        ui->unlockedGroup->setVisible(false);
    }
    else
    {
        imageUrl = data.imageUnlockedUrl;
        ui->unlockedDateLabel->setText(systemLocale.toString(unlockedDate, QLocale::ShortFormat));
        ui->unlockedGroup->setMinimumWidth(ui->unlockedDateLabel->width());
        ui->unlockedDateLabel->setVisible(false);
    }

    QString rarityText;
    if (data.rarityLevelSlug == "common")
    {
        rarityText = "Common";
    }
    else if (data.rarityLevelSlug == "uncommon")
    {
        rarityText = "Uncommon";
    }
    else if (data.rarityLevelSlug == "rare")
    {
        rarityText = "Rare";
    }
    else if (data.rarityLevelSlug == "legendary")
    {
        rarityText = "Legendary";
    }
    else if (data.rarityLevelSlug == "epic")
    {
        rarityText = "Epic";
    }
    else
    {
        rarityText = "Unknown";
    }
    ui->rarityLabel->setText(rarityText);
    ui->statisticsLabel->setText(QString("%1%2").arg(systemLocale.toString(data.rarity), systemLocale.percent()));
    if (!imageUrl.isNull())
    {
        QNetworkReply *imageReply = apiClient->getAnything(imageUrl);
        connect(this, &QObject::destroyed, imageReply, &QNetworkReply::abort);
        connect(imageReply, &QNetworkReply::finished, this, [this, imageReply]()
        {
            if (imageReply->error() == QNetworkReply::NoError)
            {
                QPixmap image;
                image.loadFromData(imageReply->readAll());
                ui->imageLabel->setPixmap(image.scaled(ui->imageLabel->size()));
            }
            else if (imageReply->error() != QNetworkReply::OperationCanceledError)
            {
                qDebug() << imageReply->error()
                         << imageReply->errorString()
                         << QString(imageReply->readAll()).toUtf8();
            }
        });
        connect(imageReply, &QNetworkReply::finished, imageReply, &QNetworkReply::deleteLater);
    }
}

AchievementListItem::~AchievementListItem()
{
    delete ui;
}

void AchievementListItem::enterEvent(QEnterEvent* event)
{
    ui->unlockedDateLabel->setVisible(true);
}

void AchievementListItem::leaveEvent(QEvent* event)
{
    ui->unlockedDateLabel->setVisible(false);
}
