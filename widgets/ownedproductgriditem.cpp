#include "ownedproductgriditem.h"
#include "ui_ownedproductgriditem.h"

OwnedProductGridItem::OwnedProductGridItem(const api::OwnedProduct &data,
                                           api::GogApiClient *apiClient,
                                           QWidget *parent) :
    QWidget(parent),
    ui(new Ui::OwnedProductGridItem)
{
    ui->setupUi(this);

    ui->titleLabel->setText(data.title);

    ui->titleLabel->setVisible(false);
    ui->ratingLabel->setVisible(false);
    ui->statusLabel->setVisible(false);
    ui->additionalInfoLabel->setVisible(false);

    imageReply = apiClient->getAnything(QString("https:%1_glx_logo_2x.png").arg(data.image));
    connect(imageReply, &QNetworkReply::finished, this, [this]() {
        if (imageReply->error() == QNetworkReply::NoError)
        {
            QPixmap image;
            image.loadFromData(imageReply->readAll());
            ui->coverLabel->setPixmap(image.scaled(ui->coverLabel->size()));
        }
        imageReply->deleteLater();
        imageReply = nullptr;
    });

    itemId = data.id;
}

OwnedProductGridItem::~OwnedProductGridItem()
{
    if (imageReply != nullptr)
    {
        imageReply->deleteLater();
    }
    delete ui;
}

void OwnedProductGridItem::mousePressEvent(QMouseEvent *event)
{
    emit navigateToProduct(itemId);
}
