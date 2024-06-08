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
    setFixedSize(size);
    thumbnailReply = apiClient->getAnything(data.thumbnailLink);
    connect(thumbnailReply, &QNetworkReply::finished, this, [this]() {
        auto networkReply = thumbnailReply;
        thumbnailReply = nullptr;
        if (networkReply->error() == QNetworkReply::NoError)
        {
            QPixmap image;
            image.loadFromData(networkReply->readAll());
            QPalette backgroundPalette;
            backgroundPalette.setBrush(this->backgroundRole(), QBrush(image.scaled(this->size(), Qt::IgnoreAspectRatio)));
            this->setAutoFillBackground(true);
            this->setPalette(backgroundPalette);
        }
        networkReply->deleteLater();
    });
}

VideoHolder::~VideoHolder()
{
    if (thumbnailReply != nullptr)
    {
        thumbnailReply->abort();
    }
    delete ui;
}

void VideoHolder::mousePressEvent(QMouseEvent *event)
{
    emit clicked();
}

