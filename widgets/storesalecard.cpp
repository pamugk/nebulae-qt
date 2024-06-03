#include "storesalecard.h"
#include "ui_storesalecard.h"

#include <QNetworkReply>

StoreSaleCard::StoreSaleCard(const api::StoreNowOnSaleTab &data,
                             api::GogApiClient *apiClient,
                             QWidget *parent) :
    QWidget(parent),
    ui(new Ui::StoreSaleCard)
{
    ui->setupUi(this);

    auto systemLocale = QLocale::system();
    ui->titleLabel->setText(data.bigThingy.text);
    ui->upToLabel->setVisible(data.bigThingy.discountUpTo);
    ui->discountLabel->setText(QString("-%1%2").arg(data.bigThingy.discountValue).arg(systemLocale.percent()));
    ui->endLabel->setText(systemLocale.toString(data.bigThingy.countdownDate, QLocale::ShortFormat));
    imageReply = apiClient->getAnything(data.bigThingy.background);
    connect(imageReply, &QNetworkReply::finished, this, [this]() {
        auto networkReply = imageReply;
        imageReply = nullptr;
        if (networkReply->error() == QNetworkReply::NoError)
        {
            QPixmap image;
            // TODO: somehow display bacckground image
            image.loadFromData(networkReply->readAll());
            this->update();
        }
        networkReply->deleteLater();
    });
}

StoreSaleCard::~StoreSaleCard()
{
    if (imageReply != nullptr)
    {
        imageReply->abort();
    }
    delete ui;
}

void StoreSaleCard::mousePressEvent(QMouseEvent *event)
{
    emit navigateToItem();
}
