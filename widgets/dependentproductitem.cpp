#include "dependentproductitem.h"
#include "ui_dependentproductitem.h"

DependentProductItem::DependentProductItem(const api::GetCatalogProductInfoResponse &data,
                                           api::GogApiClient *apiClient,
                                           QWidget *parent) :
    QWidget(parent),
    ui(new Ui::DependentProductItem)
{
    ui->setupUi(this);

    ui->titleLabel->setText(data.title);
    imageReply = apiClient->getAnything(data.imageLink.templated
                                        ? QString(data.imageLink.href).replace("{formatter}", "glx_logo")
                                        : data.imageLink.href);
    connect(imageReply, &QNetworkReply::finished, this, [this]()
    {
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

DependentProductItem::~DependentProductItem()
{
    delete ui;
}

void DependentProductItem::switchUiAuthenticatedState(bool authenticated)
{
    ui->addToCartButton->setEnabled(authenticated);
}

void DependentProductItem::mousePressEvent(QMouseEvent *event)
{
    emit clicked();
}
