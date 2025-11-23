#include "imageholder.h"
#include "ui_imageholder.h"

#include <QNetworkReply>

ImageHolder::ImageHolder(QSize size,
                         const QString &url,
                         api::GogApiClient *apiClient,
                         QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ImageHolder)
{
    ui->setupUi(this);
    setFixedSize(size);
    QNetworkReply *imageReply = apiClient->getAnything(url);
    connect(this, &QObject::destroyed, imageReply, &QNetworkReply::abort);
    connect(imageReply, &QNetworkReply::finished, this, [this, imageReply]()
    {
        if (imageReply->error() == QNetworkReply::NoError)
        {
            QPixmap image;
            image.loadFromData(imageReply->readAll());
            QPalette backgroundPalette;
            backgroundPalette.setBrush(this->backgroundRole(), QBrush(image.scaled(this->size(), Qt::IgnoreAspectRatio)));
            this->setAutoFillBackground(true);
            this->setPalette(backgroundPalette);
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

ImageHolder::~ImageHolder()
{
    delete ui;
}

void ImageHolder::mousePressEvent(QMouseEvent *event)
{
    emit clicked();
}
