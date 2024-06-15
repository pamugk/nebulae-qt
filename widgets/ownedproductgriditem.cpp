#include "ownedproductgriditem.h"
#include "ui_ownedproductgriditem.h"

void setElidedText(QLabel *label, const QString &text)
{
    QFontMetrics metrics(label->font());
    QString elidedText = metrics.elidedText(text, Qt::ElideRight, label->width());
    label->setText(elidedText);
}

OwnedProductGridItem::OwnedProductGridItem(const api::OwnedProduct &data,
                                           api::GogApiClient *apiClient,
                                           QWidget *parent) :
    QWidget(parent),
    company(""),
    genres(""),
    title(data.title),
    ui(new Ui::OwnedProductGridItem)
{
    ui->setupUi(this);

    setElidedText(ui->titleLabel, title);

    ui->titleLabel->setVisible(false);
    ui->statusActionButton->setVisible(false);
    ui->ratingLabel->setVisible(false);
    ui->additionalInfoLabel->setVisible(false);

    imageReply = apiClient->getAnything(QString("https:%1_glx_logo_2x.webp").arg(data.image));
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
