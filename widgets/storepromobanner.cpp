#include "storepromobanner.h"
#include "ui_storepromobanner.h"

StorePromoBanner::StorePromoBanner(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::StorePromoBanner)
{
    ui->setupUi(this);
    connect(ui->customInfoButton, &QPushButton::clicked, this, [this]()
    {
        emit customInfoClicked();
    });
}

StorePromoBanner::~StorePromoBanner()
{
    delete ui;
}

void StorePromoBanner::setBackgroundImage(const QPixmap &image)
{
    QPalette backgroundPalette;
    backgroundPalette.setBrush(this->backgroundRole(), QBrush(image.scaled(this->size(), Qt::KeepAspectRatioByExpanding)));
    this->setAutoFillBackground(true);
    this->setPalette(backgroundPalette);
}

void StorePromoBanner::setCustomButton(const QString &text)
{
    ui->customInfoButton->setText(text);
}
