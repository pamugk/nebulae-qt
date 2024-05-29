#include "imageholder.h"
#include "ui_imageholder.h"

ImageHolder::ImageHolder(QSize size,
                         const api::FormattedLink &data,
                         api::GogApiClient *apiClient,
                         QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ImageHolder)
{
    ui->setupUi(this);
    ui->imageLabel->setFixedSize(size);
    QString url = data.href;
    if (data.templated)
    {
        url.replace("{formatter}", data.formatters[1]);
    }
    imageReply = apiClient->getAnything(url);
    connect(imageReply, &QNetworkReply::finished, this, [this]() {
        auto networkReply = imageReply;
        imageReply = nullptr;
        if (networkReply->error() == QNetworkReply::NoError)
        {
            QPixmap image;
            image.loadFromData(networkReply->readAll());
            ui->imageLabel->setPixmap(image.scaled(ui->imageLabel->size()));
        }
        networkReply->deleteLater();
    });
}

ImageHolder::~ImageHolder()
{
    if (imageReply != nullptr)
    {
        imageReply->abort();
    }
    delete ui;
}
