#include "orderitem.h"
#include "ui_orderitem.h"

#include <QLocale>

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
        auto systemLocale = QLocale::system();
        if (data.price.discounted)
        {
            ui->oldPriceLabel->setText(systemLocale.toCurrencyString(data.price.baseAmount, data.price.symbol));
        }
        else
        {
            ui->oldPriceLabel->setVisible(false);
        }
        ui->priceLabel->setText(systemLocale.toCurrencyString(data.cashValue.amount, data.cashValue.symbol));
    }
    else
    {
        ui->oldPriceLabel->setVisible(false);
        ui->priceLabel->setText(data.status.toUpper());
    }

    imageReply = apiClient->getAnything(QString("https:%1_100.png").arg(data.image));
    connect(imageReply, &QNetworkReply::finished, this, [this]() {
        auto networkReply = imageReply;
        imageReply = nullptr;
        if (networkReply->error() == QNetworkReply::NoError)
        {
            QPixmap image;
            image.loadFromData(networkReply->readAll());
            ui->coverLabel->setPixmap(image);
        }
        networkReply->deleteLater();
    });
}

OrderItem::~OrderItem()
{
    if (imageReply != nullptr)
    {
        imageReply->abort();
    }
    delete ui;
}

void OrderItem::mousePressEvent(QMouseEvent *event)
{
    emit clicked();
}
