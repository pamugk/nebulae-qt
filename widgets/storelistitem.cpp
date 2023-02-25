#include "storelistitem.h"
#include "ui_storelistitem.h"

StoreListItem::StoreListItem(const CatalogProduct &data,
                             GogApiClient *apiClient,
                             QWidget *parent) :
    QWidget(parent),
    ui(new Ui::StoreListItem)
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
    imageReply = apiClient->getAnything(data.coverVertical);
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
}

StoreListItem::~StoreListItem()
{
    if (imageReply != nullptr)
    {
        imageReply->deleteLater();
    }
    delete ui;
}
