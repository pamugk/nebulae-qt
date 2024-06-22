#include "ownedproductgriditem.h"
#include "ui_ownedproductgriditem.h"

void setElidedText(QLabel *label, const QString &text)
{
    QFontMetrics metrics(label->font());
    QString elidedText = metrics.elidedText(text, Qt::ElideRight, label->width());
    label->setText(elidedText);
}

OwnedProductGridItem::OwnedProductGridItem(const api::Release &data,
                                           api::GogApiClient *apiClient,
                                           QWidget *parent) :
    QWidget(parent),
    company(""),
    genres(""),
    title(data.title["*"].toString()),
    ui(new Ui::OwnedProductGridItem)
{
    ui->setupUi(this);

    setElidedText(ui->titleLabel, title);

    ui->coverLabel->setToolTip(title);
    ui->titleLabel->setVisible(false);
    ui->titleLabel->setToolTip(title);
    ui->statusActionButton->setVisible(false);
    ui->ratingLabel->setVisible(false);
    ui->additionalInfoLabel->setVisible(false);

    imageReply = apiClient->getAnything(QString(data.game.verticalCover).replace("{formatter}", "_product_tile_80x114_2x").replace("{ext}", "webp"));
    connect(imageReply, &QNetworkReply::finished, this, [this]()
    {
        auto networkReply = imageReply;
        imageReply = nullptr;
        if (networkReply->error() == QNetworkReply::NoError)
        {
            QPixmap image;
            image.loadFromData(networkReply->readAll());
            ui->coverLabel->setPixmap(image);
        }
        networkReply->deleteLater();
    });
}

OwnedProductGridItem::~OwnedProductGridItem()
{
    if (imageReply != nullptr)
    {
        imageReply->abort();
    }
    delete ui;
}

void OwnedProductGridItem::setAdditionalDataVisibility(bool visible)
{
    ui->additionalInfoLabel->setVisible(visible);
}

void OwnedProductGridItem::setAdditionalDataDisplayed(int kind)
{
    switch (kind)
    {
    case COMPANY:
        ui->additionalInfoLabel->setText(company);
        break;
    case GENRES:
        ui->additionalInfoLabel->setText(genres);
        break;
    case STATS:
        ui->additionalInfoLabel->setText("");
        break;
    case TAGS:
        ui->additionalInfoLabel->setText("");
        break;
    case PLATFORM:
        ui->additionalInfoLabel->setText("GOG");
        break;
    }
}

void OwnedProductGridItem::setImageSize(const QSize &imageSize)
{
    this->setFixedWidth(imageSize.width());
    ui->coverLabel->setFixedSize(imageSize);
}

void OwnedProductGridItem::setRatingVisibility(bool visible)
{
    ui->ratingLabel->setVisible(visible);
}

void OwnedProductGridItem::setStatusVisibility(bool visible)
{
    ui->statusActionButton->setVisible(visible);
    setElidedText(ui->titleLabel, title);
}

void OwnedProductGridItem::setTitleVisibility(bool visible)
{
    ui->titleLabel->setVisible(visible);
}

void OwnedProductGridItem::mousePressEvent(QMouseEvent *event)
{
    emit clicked();
}
