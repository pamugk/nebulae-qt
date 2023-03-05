#include "storegridtile.h"
#include "ui_storegridtile.h"

StoreGridTile::StoreGridTile(const api::CatalogProduct &data,
                             api::GogApiClient *apiClient,
                             QWidget *parent) :
    QWidget(parent),
    ui(new Ui::StoreGridTile)
{
    ui->setupUi(this);
    ui->titleLabel->setText(data.title);
    if (data.price.discount.isNull())
    {
        ui->discountLabel->setVisible(false);
        ui->oldPriceLabel->setVisible(false);
    }
    else
    {
        ui->discountLabel->setText(data.price.discount);
        ui->oldPriceLabel->setText(data.price.base);
    }
    ui->priceLabel->setText(data.price.final);
    imageReply = apiClient->getAnything(data.coverHorizontal);
    connect(imageReply, &QNetworkReply::finished, this, [this]() {
        if (imageReply->error() == QNetworkReply::NoError)
        {
            QPixmap image;
            image.loadFromData(imageReply->readAll());
            ui->coverLabel->setPixmap(image.scaled(ui->coverLabel->size()));
        }
        imageReply->deleteLater();
        imageReply = nullptr;
    });

    itemId = data.id.toLongLong();
}

StoreGridTile::~StoreGridTile()
{
    if (imageReply != nullptr)
    {
        imageReply->deleteLater();
    }
    delete ui;
}

void StoreGridTile::mousePressEvent(QMouseEvent *event)
{
    emit navigateToProduct(itemId);
}
