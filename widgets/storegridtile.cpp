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
    QNetworkReply *imageReply = apiClient->getAnything(data.coverHorizontal);
    connect(this, &StoreGridTile::destroyed, imageReply, &QNetworkReply::abort);
    connect(imageReply, &QNetworkReply::finished, this, [this, imageReply]() {
        if (imageReply->error() == QNetworkReply::NoError)
        {
            QPixmap image;
            image.loadFromData(imageReply->readAll());
            ui->coverLabel->setPixmap(image.scaled(ui->coverLabel->size()));
        }
        imageReply->deleteLater();
    });
}

StoreGridTile::~StoreGridTile()
{
    delete ui;
}

void StoreGridTile::setOwned(bool owned)
{
    ui->ownedLabel->setVisible(owned);
}

void StoreGridTile::setWishlisted(bool wishlisted)
{
    ui->wishlistedLabel->setVisible(wishlisted);
}

void StoreGridTile::switchUiAuthenticatedState(bool authenticated)
{
    ui->addToCartButton->setEnabled(authenticated);
}

void StoreGridTile::mousePressEvent(QMouseEvent *event)
{
    emit clicked();
}
