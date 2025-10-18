#include "storehighlightsitem.h"
#include "ui_storehighlightsitem.h"

#include <QDesktopServices>

StoreHighlightsItem::StoreHighlightsItem(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::StoreHighlightsItem)
{
    ui->setupUi(this);
    ui->discountLabel->setVisible(false);
    ui->oldPriceLabel->setVisible(false);
    ui->newPriceLabel->setVisible(false);
    ui->wishlistButton->setVisible(false);
    ui->addToCartButton->setVisible(false);
    ui->customInfoButton->setVisible(false);
    connect(ui->customInfoButton, &QPushButton::clicked, this, [this]()
    {
        emit customInfoClicked();
    });
}

StoreHighlightsItem::~StoreHighlightsItem()
{
    delete ui;
}

void StoreHighlightsItem::setBackgroundImage(const QPixmap &image)
{
    QPalette backgroundPalette;
    backgroundPalette.setBrush(this->backgroundRole(), QBrush(image.scaled(this->size(), Qt::KeepAspectRatioByExpanding)));
    this->setAutoFillBackground(true);
    this->setPalette(backgroundPalette);
}

void StoreHighlightsItem::setCustomButton(const QString &text)
{
    ui->customInfoButton->setText(text);
    ui->customInfoButton->setVisible(true);
}

void StoreHighlightsItem::setLogoImage(const QPixmap &image)
{
    ui->logoLabel->setPixmap(image.scaled(ui->logoLabel->size()));
}

void StoreHighlightsItem::setPrice(double basePrice, double finalPrice, unsigned char discount)
{
    auto systemLocale = QLocale::system();
    if (discount > 0)
    {
        ui->discountLabel->setVisible(true);
        ui->oldPriceLabel->setVisible(true);
        ui->discountLabel->setText(QString("%1%2%3").arg(systemLocale.negativeSign(), QString::number(discount), systemLocale.percent()));
        ui->oldPriceLabel->setText(systemLocale.toCurrencyString(basePrice, ""));
    }
    ui->newPriceLabel->setVisible(true);
    ui->newPriceLabel->setText(
                finalPrice > 0
                ? systemLocale.toCurrencyString(finalPrice, "")
                : tr("Free"));
    ui->addToCartButton->setVisible(true);
}

void StoreHighlightsItem::setSubtitle(const QString &subtitle)
{
    ui->subtitleLabel->setText(subtitle);
}

void StoreHighlightsItem::setTitle(const QString &title)
{
    ui->titleLabel->setText(title);
}

void StoreHighlightsItem::setWishlisted(bool wishlisted)
{
    ui->wishlistButton->setVisible(true);
    ui->wishlistButton->setIcon(QIcon(wishlisted ? ":icons/heart-solid.svg" : ":icons/heart.svg"));
}

void StoreHighlightsItem::mousePressEvent(QMouseEvent *event)
{
    emit clicked();
}
