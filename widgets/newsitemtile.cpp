#include "newsitemtile.h"
#include "ui_newsitemtile.h"

#include <QNetworkReply>

NewsItemTile::NewsItemTile(const api::NewsItem &data,
                           bool primary,
                           api::GogApiClient *apiClient,
                           QWidget *parent) :
    QWidget(parent),
    ui(new Ui::NewsItemTile)
{
    ui->setupUi(this);
    ui->titleLabel->setText(data.title);
    ui->commentsCountLabel->setText(QString::number(data.commentsCount));
    if (data.publishDate.isValid())
    {
        ui->publishDateLabel->setText(QLocale::system().toString(data.publishDate, QLocale::ShortFormat));
    }

    int width = primary ? 536 : 256;
    this->setMinimumWidth(width);
    this->setMaximumWidth(width);
    ui->rootFrame->setMinimumWidth(width);
    ui->rootFrame->setMaximumWidth(width);
    QNetworkReply *imageReply = apiClient->getAnything(data.imageSmall);
    connect(this, &QObject::destroyed, imageReply, &QNetworkReply::abort);
    connect(imageReply, &QNetworkReply::finished, this, [this, primary, imageReply]() {
        if (imageReply->error() == QNetworkReply::NoError)
        {
            QPixmap image;
            image.loadFromData(imageReply->readAll());
            QPalette backgroundPalette;
            backgroundPalette.setBrush(this->backgroundRole(), QBrush(primary
                                                                      ? image.scaled(this->size(), Qt::KeepAspectRatioByExpanding)
                                                                      : image.copy(150, 0, 256, 264)));
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

    itemId = data.id;
}

NewsItemTile::~NewsItemTile()
{
    delete ui;
}

void NewsItemTile::mousePressEvent(QMouseEvent *event)
{
    emit navigateToNewsItem(itemId);
}
