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
    ui->ownedLabel->setVisible(false);
    ui->upcomingLabel->setVisible(false);
    ui->wishlistedLabel->setVisible(false);
    if (data.price.has_value())
    {
        const auto &price = data.price.value();
        if (price.discount.isNull())
        {
            ui->discountLabel->setVisible(false);
            ui->oldPriceLabel->setVisible(false);
        }
        else
        {
            ui->discountLabel->setText(price.discount);
            ui->oldPriceLabel->setText(price.base);
        }
        ui->priceLabel->setText(price.final);
    }
    else
    {
        ui->discountLabel->setVisible(false);
        ui->oldPriceLabel->setVisible(false);
        ui->priceLabel->setVisible(false);
    }
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
}

StoreListItem::~StoreListItem()
{
    if (imageReply != nullptr)
    {
        imageReply->abort();
    }
    delete ui;
}

void StoreListItem::setOwned(bool owned)
{
    ui->ownedLabel->setVisible(owned);
}

void StoreListItem::setWishlisted(bool wishlisted)
{
    ui->wishlistedLabel->setVisible(wishlisted);
}

void StoreListItem::switchUiAuthenticatedState(bool authenticated)
{
    ui->addToCartButton->setEnabled(authenticated);
}

void StoreListItem::mousePressEvent(QMouseEvent *event)
{
    emit clicked();
}
