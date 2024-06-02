#include "simpleproductitem.h"
#include "ui_simpleproductitem.h"

SimpleProductItem::SimpleProductItem(unsigned long long productId,
                                     QWidget *parent) :
    QWidget(parent),
    itemId(productId),
    imageReply(nullptr),
    ui(new Ui::SimpleProductItem)
{
    ui->setupUi(this);

    ui->discountLabel->setVisible(false);
    ui->oldPriceLabel->setVisible(false);
    ui->newPriceLabel->setVisible(false);
}

SimpleProductItem::~SimpleProductItem()
{
    if (imageReply != nullptr)
    {
        imageReply->abort();
    }
    delete ui;
}

void SimpleProductItem::setCover(const QString &coverUrl, api::GogApiClient *apiClient)
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

void SimpleProductItem::setPrice(unsigned int basePrice, unsigned int finalPrice,
                                 const QString &currency)
{

    auto systemLocale = QLocale::system();
    if (basePrice != finalPrice)
    {
        ui->discountLabel->setText(QString("-%1%2")
                                   .arg(round(100. * (basePrice - finalPrice) / basePrice))
                                   .arg(systemLocale.percent()));
        ui->discountLabel->setVisible(true);
        ui->oldPriceLabel->setText(systemLocale.toCurrencyString(basePrice / 100., currency));
        ui->oldPriceLabel->setVisible(true);
    }
    ui->newPriceLabel->setText(
                finalPrice == 0
                ? "Free"
                : systemLocale.toCurrencyString(finalPrice / 100, currency));
    ui->newPriceLabel->setVisible(true);
}

void SimpleProductItem::setTitle(const QString &title)
{
    ui->titleLabel->setText(title);
}

void SimpleProductItem::switchUiAuthenticatedState(bool authenticated)
{
    ui->wishlistButton->setEnabled(authenticated);
    ui->addToCartButton->setEnabled(authenticated);
}

void SimpleProductItem::mousePressEvent(QMouseEvent *event)
{
    emit navigateToProduct(itemId);
}
