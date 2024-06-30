#include "videoholder.h"
#include "ui_videoholder.h"

#include <QNetworkReply>

VideoHolder::VideoHolder(QSize size,
                         const QString &thumbnailLink,
                         api::GogApiClient *apiClient,
                         QWidget *parent) :
    QWidget(parent),
    ui(new Ui::VideoHolder)
{
    ui->setupUi(this);
    setFixedSize(size);
    QNetworkReply *thumbnailReply = apiClient->getAnything(thumbnailLink);
    connect(thumbnailReply, &QNetworkReply::finished, this, [this, thumbnailReply]()
    {
        if (thumbnailReply->error() == QNetworkReply::NoError)
        {
            QPixmap image;
            image.loadFromData(thumbnailReply->readAll());
            QPalette backgroundPalette;
            backgroundPalette.setBrush(this->backgroundRole(), QBrush(image.scaled(this->size(), Qt::IgnoreAspectRatio)));
            this->setAutoFillBackground(true);
            this->setPalette(backgroundPalette);
        }
        thumbnailReply->deleteLater();
    });
    connect(this, &QObject::destroyed, thumbnailReply, &QNetworkReply::abort);
}

VideoHolder::~VideoHolder()
{
    delete ui;
}

void VideoHolder::mousePressEvent(QMouseEvent *event)
{
    emit clicked();
}

