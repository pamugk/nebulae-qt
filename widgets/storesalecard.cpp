#include "storesalecard.h"
#include "ui_storesalecard.h"

#include <QNetworkReply>

StoreSaleCard::StoreSaleCard(const api::StoreNowOnSaleTabCard &data,
                             api::GogApiClient *apiClient,
                             QWidget *parent) :
    QWidget(parent),
    ui(new Ui::StoreSaleCard)
{
    ui->setupUi(this);

    auto systemLocale = QLocale::system();
    ui->titleLabel->setText(data.text);
    ui->upToLabel->setVisible(data.discountUpTo);
    ui->discountLabel->setText(QString("%1%2%3").arg(systemLocale.negativeSign(), QString::number(data.discountValue), systemLocale.percent()));
    ui->endLabel->setText(systemLocale.toString(data.countdownDate, QLocale::ShortFormat));

    ui->content->setStyleSheet(QString("background: rgba(%1, %2, %3, 0.75)")
                               .arg(data.colorRgbArray[0]).arg(data.colorRgbArray[1]).arg(data.colorRgbArray[2]));
    /*QPalette shadowPalette;
    shadowPalette.setBrush(ui->content->backgroundRole(), QColor::fromRgba64(data.colorRgbArray[0], data.colorRgbArray[1], data.colorRgbArray[2], 191));
    ui->content->setAutoFillBackground(true);
    ui->content->setPalette(shadowPalette);*/
    QNetworkReply *imageReply = apiClient->getAnything(data.background);
    connect(this, &StoreSaleCard::destroyed, imageReply, &QNetworkReply::abort);
    connect(imageReply, &QNetworkReply::finished, this, [this, imageReply]() {
        if (imageReply->error() == QNetworkReply::NoError)
        {
            QPixmap image;
            image.loadFromData(imageReply->readAll());
            QPalette backgroundPalette;
            backgroundPalette.setBrush(this->backgroundRole(), QBrush(image.scaled(this->size(), Qt::KeepAspectRatioByExpanding)));
            this->setAutoFillBackground(true);
            this->setPalette(backgroundPalette);
        }
        imageReply->deleteLater();
    });
}

StoreSaleCard::~StoreSaleCard()
{
    delete ui;
}

void StoreSaleCard::mousePressEvent(QMouseEvent *event)
{
    emit navigateToItem();
}
