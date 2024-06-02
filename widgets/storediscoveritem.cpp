#include "storediscoveritem.h"
#include "ui_storediscoveritem.h"

#include <QNetworkReply>

StoreDiscoverItem::StoreDiscoverItem(unsigned long long id, QWidget *parent) :
    QWidget(parent),
    id(id),
    ui(new Ui::StoreDiscoverItem)
{
    ui->setupUi(this);

    ui->comingSoonLabel->setVisible(false);
}

StoreDiscoverItem::~StoreDiscoverItem()
{
    if (imageReply != nullptr)
    {
        imageReply->abort();
    }
    delete ui;
}

void StoreDiscoverItem::setCover(const QString &coverUrl, api::GogApiClient *apiClient)
{
    imageReply = apiClient->getAnything(coverUrl);
    connect(imageReply, &QNetworkReply::finished, this, [this]() {
        auto networkReply = imageReply;
        imageReply = nullptr;
        if (networkReply->error() == QNetworkReply::NoError)
        {
            QPixmap image;
            image.loadFromData(networkReply->readAll());
            ui->coverLabel->setPixmap(image.scaled(ui->coverLabel->size(), Qt::KeepAspectRatioByExpanding));
        }
        networkReply->deleteLater();
    });
}

void StoreDiscoverItem::setPreorder(bool preorder)
{
    ui->comingSoonLabel->setVisible(preorder);
}

void StoreDiscoverItem::setTitle(const QString &title)
{
    ui->titleLabel->setText(title);
}

void StoreDiscoverItem::setPrice(double basePrice, double finalPrice,
                                 unsigned char discount, bool free, const QString &currency)
{
    auto systemLocale = QLocale::system();
    if (discount > 0)
    {
        ui->discountLabel->setText(QString("-%1%2")
                                   .arg(discount)
                                   .arg(systemLocale.percent()));
        ui->oldPriceLabel->setText(systemLocale.toCurrencyString(basePrice, currency));
    }
    else
    {
        ui->discountLabel->setVisible(false);
        ui->oldPriceLabel->setVisible(false);
    }
    ui->newPriceLabel->setText(
                free
                ? "Free"
                : systemLocale.toCurrencyString(finalPrice, currency));
}

void StoreDiscoverItem::mousePressEvent(QMouseEvent *event)
{
    emit navigateToProduct(id);
}
