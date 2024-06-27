#include "releasechangelogdialog.h"
#include "ui_releasechangelogdialog.h"

#include <QNetworkReply>

ReleaseChangelogDialog::ReleaseChangelogDialog(const QString &changelog,
                                               const QString &title,
                                               const QString &platform,
                                               const QString &iconLink,
                                               api::GogApiClient *apiClient,
                                               QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ReleaseChangelogDialog)
{
    ui->setupUi(this);

    ui->titleLabel->setText(title);
    ui->platformLabel->setText(platform);
    ui->changelogBrowser->setText(changelog);

    auto imageReply = apiClient->getAnything(iconLink);
    connect(imageReply, &QNetworkReply::finished, this, [this, imageReply]()
    {
        if (imageReply->error() == QNetworkReply::NoError)
        {
            QPixmap image;
            image.loadFromData(imageReply->readAll());
            ui->iconLabel->setPixmap(image);
        }
        imageReply->deleteLater();
    });
}

ReleaseChangelogDialog::~ReleaseChangelogDialog()
{
    delete ui;
}
