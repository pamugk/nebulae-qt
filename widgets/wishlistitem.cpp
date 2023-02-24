#include "wishlistitem.h"
#include "ui_wishlistitem.h"

#include <QSvgWidget>

WishlistItem::WishlistItem(const Product &data, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::WishlistItem)
{
    ui->setupUi(this);

    ui->titleLabel->setText(data.title);
    ui->genreLabel->setText(data.category);
    ui->buyButton->setText(data.TBA ? "TBA" : QString("%1 %2").arg(data.price.amount, data.price.symbol));

    int star = 1;
    for (star; star <= data.rating / 10; star++)
    {
        auto starWidget = new QSvgWidget(":/icons/star.svg", ui->rating);
        starWidget->setFixedSize(16, 16);
        ui->ratingLayout->addWidget(starWidget);
    }
    if (data.rating % 10 >= 5)
    {
        star++;
        auto starWidget = new QSvgWidget(":/icons/star-half-stroke.svg", ui->rating);
        starWidget->setFixedSize(16, 16);
        ui->ratingLayout->addWidget(starWidget);
    }
    for (star; star <= 5; star++)
    {
        auto starWidget = new QSvgWidget(":/icons/star-stroke.svg", ui->rating);
        starWidget->setFixedSize(16, 16);
        ui->ratingLayout->addWidget(starWidget);
    }
}

WishlistItem::~WishlistItem()
{
    delete ui;
}
