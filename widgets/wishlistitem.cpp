#include "wishlistitem.h"
#include "ui_wishlistitem.h"

WishlistItem::WishlistItem(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::WishlistItem)
{
    ui->setupUi(this);
}

WishlistItem::~WishlistItem()
{
    delete ui;
}
