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
    imageReply = apiClient->getAnything(data.imageSmall);
    connect(imageReply, &QNetworkReply::finished, this, [this, primary]() {
        auto networkReply = imageReply;
        imageReply = nullptr;
        if (networkReply->error() == QNetworkReply::NoError)
        {
            QPixmap image;
            image.loadFromData(networkReply->readAll());
            QPalette backgroundPalette;
            backgroundPalette.setBrush(this->backgroundRole(), QBrush(primary
                                                                      ? image.scaled(this->size(), Qt::KeepAspectRatioByExpanding)
                                                                      : image.copy(150, 0, 256, 264)));
            this->setAutoFillBackground(true);
            this->setPalette(backgroundPalette);
        }
        networkReply->deleteLater();
    });

    itemId = data.id;
}

NewsItemTile::~NewsItemTile()
{
    if (imageReply != nullptr)
    {
        imageReply->abort();
    }
    delete ui;
}

void NewsItemTile::mousePressEvent(QMouseEvent *event)
{
    emit navigateToNewsItem(itemId);
}
