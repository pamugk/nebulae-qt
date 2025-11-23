#include "wishlistitem.h"
#include "ui_wishlistitem.h"

#include <QSvgWidget>

WishlistItem::WishlistItem(const api::Product &data,
                           api::GogApiClient *apiClient,
                           QWidget *parent) :
    QWidget(parent),
    ui(new Ui::WishlistItem)
{
    ui->setupUi(this);

    ui->titleLabel->setText(data.title);
    ui->genreLabel->setText(data.category);
    ui->buyButton->setText(data.TBA ? "TBA" : QString("%1 %2").arg(data.price.amount, data.price.symbol));

    int star = 1;
    for (; star <= data.rating / 10; star++)
    {
        auto starWidget = new QSvgWidget(":/icons/star.svg", ui->rating);
        starWidget->setFixedSize(16, 16);
        ui->ratingLayout->addWidget(starWidget);
    }
    if (data.rating % 10 >= 5)
    {
        star++;
        auto starWidget = new QSvgWidget(":/icons/star-half-stroke.svg", ui->rating);
        starWidget->setFixedSize(16, 16);
        ui->ratingLayout->addWidget(starWidget);
    }
    for (; star <= 5; star++)
    {
        auto starWidget = new QSvgWidget(":/icons/star-stroke.svg", ui->rating);
        starWidget->setFixedSize(16, 16);
        ui->ratingLayout->addWidget(starWidget);
    }

    QNetworkReply *imageReply = apiClient->getAnything(QString("https:%1_100.png").arg(data.image));
    connect(this, &QObject::destroyed, imageReply, &QNetworkReply::abort);
    connect(imageReply, &QNetworkReply::finished, this, [this, imageReply]() {
        if (imageReply->error() == QNetworkReply::NoError)
        {
            ui->coverLabel->setPixmap(QPixmap::fromImage(QImage::fromData(imageReply->readAll(), "PNG")));
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

WishlistItem::~WishlistItem()
{
    delete ui;
}

void WishlistItem::mousePressEvent(QMouseEvent *event)
{
    emit clicked();
}
