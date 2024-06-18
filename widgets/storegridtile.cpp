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

StoreGridTile::~StoreGridTile()
{
    if (imageReply != nullptr)
    {
        imageReply->abort();
    }
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
