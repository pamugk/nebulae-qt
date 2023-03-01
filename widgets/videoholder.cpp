#include "videoholder.h"
#include "ui_videoholder.h"

VideoHolder::VideoHolder(QSize size,
                         const api::ThumbnailedVideo &data,
                         api::GogApiClient *apiClient,
                         QWidget *parent) :
    QWidget(parent),
    ui(new Ui::VideoHolder)
{
    ui->setupUi(this);
    ui->thumbnailLabel->setFixedSize(size);
    thumbnailReply = apiClient->getAnything(data.thumbnailLink);
    connect(thumbnailReply, &QNetworkReply::finished, this, [this]() {
        if (thumbnailReply->error() == QNetworkReply::NoError)
        {
            QPixmap image;
            image.loadFromData(thumbnailReply->readAll());
            ui->thumbnailLabel->setPixmap(image.scaled(ui->thumbnailLabel->size()));
        }
        thumbnailReply->deleteLater();
        thumbnailReply = nullptr;
    });
}

VideoHolder::~VideoHolder()
{
    if (thumbnailReply != nullptr)
    {
        thumbnailReply->deleteLater();
    }
    delete ui;
}
