#include "orderitem.h"
#include "ui_orderitem.h"

OrderItem::OrderItem(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::OrderItem)
{
    ui->setupUi(this);
}

OrderItem::~OrderItem()
{
    delete ui;
}
