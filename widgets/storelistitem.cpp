#include "storelistitem.h"
#include "ui_storelistitem.h"

StoreListItem::StoreListItem(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::StoreListItem)
{
    ui->setupUi(this);
}

StoreListItem::~StoreListItem()
{
    delete ui;
}
