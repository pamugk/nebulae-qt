#include "orderitem.h"
#include "ui_orderitem.h"

OrderItem::OrderItem(const api::OrderProduct &data,
                     api::GogApiClient *apiClient,
                     QWidget *parent) :
    QWidget(parent),
    ui(new Ui::OrderItem)
{
    ui->setupUi(this);

    ui->titleLabel->setText(data.title);
    if (data.status.isNull())
    {
        if (data.price.discounted)
        {
            ui->oldPriceLabel->setText(QString("%1 %2").arg(data.price.symbol, data.price.baseAmount));
        }
        else
        {
            ui->oldPriceLabel->setVisible(false);
        }
        ui->priceLabel->setText(data.cashValue.full);
    }
    else
    {
        ui->oldPriceLabel->setVisible(false);
        ui->priceLabel->setText(data.status.toUpper());
    }

    imageReply = apiClient->getAnything(QString("https:%1_100.png").arg(data.image));
    connect(imageReply, &QNetworkReply::finished, this, [this]() {
        if (imageReply->error() == QNetworkReply::NoError)
        {
            QPixmap image;
            image.loadFromData(imageReply->readAll());
            ui->coverLabel->setPixmap(image);
        }
        imageReply->deleteLater();
        imageReply = nullptr;
    });
}

OrderItem::~OrderItem()
{
    if (imageReply != nullptr)
    {
        imageReply->deleteLater();
    }
    delete ui;
}
