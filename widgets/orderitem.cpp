#include "orderitem.h"
#include "ui_orderitem.h"

#include <QLocale>
#include <QNetworkReply>

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

    QNetworkReply *imageReply = apiClient->getAnything(QString("https:%1_100.png").arg(data.image));
    connect(this, &QObject::destroyed, imageReply, &QNetworkReply::abort);
    connect(imageReply, &QNetworkReply::finished, this, [this, imageReply]() {
        if (imageReply->error() == QNetworkReply::NoError)
        {
            QPixmap image;
            image.loadFromData(imageReply->readAll());
            ui->coverLabel->setPixmap(image);
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

OrderItem::~OrderItem()
{
    delete ui;
}

void OrderItem::mousePressEvent(QMouseEvent *event)
{
    emit clicked();
}
