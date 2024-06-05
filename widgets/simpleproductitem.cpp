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
            this->setMinimumWidth(image.width());
            this->setMaximumWidth(this->minimumWidth());
            this->setMinimumHeight(image.height() + 84);
            this->setMaximumHeight(this->minimumHeight());
            ui->coverLabel->setMinimumSize(image.size());
            ui->coverLabel->setMaximumSize(image.size());
            ui->coverLabel->setPixmap(image);
        }
        networkReply->deleteLater();
    });
}

void SimpleProductItem::setPrice(double basePrice, double finalPrice,
                                 unsigned char discount, bool free, const QString &currency)
{
    auto systemLocale = QLocale::system();
    if (discount > 0)
    {
        ui->discountLabel->setText(QString("-%1%2")
                                   .arg(discount)
                                   .arg(systemLocale.percent()));
        ui->discountLabel->setVisible(true);
        ui->oldPriceLabel->setText(systemLocale.toCurrencyString(basePrice, currency));
        ui->oldPriceLabel->setVisible(true);
    }
    ui->newPriceLabel->setText(
                free
                ? "Free"
                : systemLocale.toCurrencyString(finalPrice, currency));
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
