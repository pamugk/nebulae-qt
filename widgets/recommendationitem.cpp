#include "recommendationitem.h"
#include "ui_recommendationitem.h"

RecommendationItem::RecommendationItem(const api::Recommendation &data,
                                       api::GogApiClient *apiClient,
                                       QWidget *parent) :
    QWidget(parent),
    ui(new Ui::RecommendationItem)
{
    ui->setupUi(this);

    auto systemLocale = QLocale::system();
    ui->titleLabel->setText(data.details.title);
    if (data.pricing.basePrice == data.pricing.finalPrice)
    {
        ui->discountLabel->setVisible(false);
        ui->oldPriceLabel->setVisible(false);
    }
    else
    {
        ui->discountLabel->setText(QString("-%1%2")
                                   .arg(round(100. * (data.pricing.basePrice - data.pricing.finalPrice) / data.pricing.basePrice))
                                   .arg(systemLocale.percent()));
        ui->oldPriceLabel->setText(systemLocale.toCurrencyString(data.pricing.basePrice * 1. / 100, data.pricing.currency));
    }
    ui->newPriceLabel->setText(
                data.pricing.finalPrice == 0
                ? "Free"
                : systemLocale.toCurrencyString(data.pricing.finalPrice * 1. / 100, data.pricing.currency));
    imageReply = apiClient->getAnything(data.details.imageHorizontalUrl);
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

    itemId = data.productId;
}

RecommendationItem::~RecommendationItem()
{
    if (imageReply != nullptr)
    {
        imageReply->abort();
    }
    delete ui;
}

void RecommendationItem::mousePressEvent(QMouseEvent *event)
{
    emit navigateToProduct(itemId);
}
