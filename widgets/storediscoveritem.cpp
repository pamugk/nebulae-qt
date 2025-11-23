#include "storediscoveritem.h"
#include "ui_storediscoveritem.h"

#include <QNetworkReply>

StoreDiscoverItem::StoreDiscoverItem(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::StoreDiscoverItem)
{
    ui->setupUi(this);

    ui->discountLabel->setVisible(false);
    ui->oldPriceLabel->setVisible(false);
    ui->newPriceLabel->setVisible(false);
    ui->ownedLabel->setVisible(false);
    ui->wishlistedLabel->setVisible(false);
}

StoreDiscoverItem::~StoreDiscoverItem()
{
    delete ui;
}

void StoreDiscoverItem::setCover(const QString &coverUrl, api::GogApiClient *apiClient)
{
    QNetworkReply *imageReply = apiClient->getAnything(coverUrl);
    connect(this, &QObject::destroyed, imageReply, &QNetworkReply::abort);
    connect(imageReply, &QNetworkReply::finished, this, [this, imageReply]() {
        if (imageReply->error() == QNetworkReply::NoError)
        {
            QPixmap image;
            image.loadFromData(imageReply->readAll());
            ui->coverLabel->setPixmap(image.scaled(ui->coverLabel->size(), Qt::KeepAspectRatioByExpanding));
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

void StoreDiscoverItem::setOwned(bool owned)
{
    ui->ownedLabel->setVisible(owned);
}

void StoreDiscoverItem::setTitle(const QString &title)
{
    ui->titleLabel->setText(title);
}

void StoreDiscoverItem::setWishlisted(bool wishlisted)
{
    ui->wishlistedLabel->setVisible(wishlisted);
}

void StoreDiscoverItem::setPrice(double basePrice, double finalPrice,
                                 unsigned char discount, bool free, const QString &currency)
{
    auto systemLocale = QLocale::system();
    if (discount > 0)
    {
        ui->discountLabel->setVisible(true);
        ui->oldPriceLabel->setVisible(true);
        ui->discountLabel->setText(QString("%1%2%3").arg(systemLocale.negativeSign(), QString::number(discount), systemLocale.percent()));
        ui->oldPriceLabel->setText(systemLocale.toCurrencyString(basePrice, currency));
    }
    ui->newPriceLabel->setVisible(true);
    ui->newPriceLabel->setText(
                free
                ? tr("Free")
                : systemLocale.toCurrencyString(finalPrice, currency));
}

void StoreDiscoverItem::switchUiAuthenticatedState(bool authenticated)
{
    ui->addToCartButton->setEnabled(authenticated);
}

void StoreDiscoverItem::mousePressEvent(QMouseEvent *event)
{
    emit clicked();
}
