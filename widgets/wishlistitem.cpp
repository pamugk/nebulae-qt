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

    imageReply = apiClient->getAnything(QString("https:%1_100.png").arg(data.image));
    connect(imageReply, &QNetworkReply::finished, this, [this]() {
        if (imageReply->error() == QNetworkReply::NoError)
        {
            ui->coverLabel->setPixmap(QPixmap::fromImage(QImage::fromData(imageReply->readAll(), "PNG")));
        }
        imageReply->deleteLater();
        imageReply = nullptr;
    });

    itemId = data.id;
}

WishlistItem::~WishlistItem()
{
    if (imageReply != nullptr)
    {
        imageReply->deleteLater();
    }
    delete ui;
}

void WishlistItem::mousePressEvent(QMouseEvent *event)
{
    emit navigateToProduct(itemId);
}
