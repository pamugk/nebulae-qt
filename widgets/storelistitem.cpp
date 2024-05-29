#include "storelistitem.h"
#include "ui_storelistitem.h"

StoreListItem::StoreListItem(const api::CatalogProduct &data,
                             api::GogApiClient *apiClient,
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
        auto networkReply = imageReply;
        imageReply = nullptr;
        if (networkReply->error() == QNetworkReply::NoError)
        {
            QPixmap image;
            image.loadFromData(networkReply->readAll());
            ui->coverLabel->setPixmap(image.scaled(ui->coverLabel->size()));
        }
        networkReply->deleteLater();
    });

    itemId = data.id.toLongLong();
}

StoreListItem::~StoreListItem()
{
    if (imageReply != nullptr)
    {
        imageReply->abort();
    }
    delete ui;
}

void StoreListItem::mousePressEvent(QMouseEvent *event)
{
    emit navigateToProduct(itemId);
}
