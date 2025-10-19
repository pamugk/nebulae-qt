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
    QNetworkReply *imageReply = apiClient->getAnything(data.imageLink.templated
                                        ? QString(data.imageLink.href).replace("{formatter}", "glx_logo")
                                        : data.imageLink.href);
    connect(this, &QObject::destroyed, imageReply, &QNetworkReply::abort);
    connect(imageReply, &QNetworkReply::finished, this, [this, imageReply]()
    {
        if (imageReply->error() == QNetworkReply::NoError)
        {
            QPixmap image;
            image.loadFromData(imageReply->readAll());
            ui->coverLabel->setPixmap(image.scaled(ui->coverLabel->size()));
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
