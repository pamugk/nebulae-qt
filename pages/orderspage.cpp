#include "orderspage.h"
#include "ui_orderspage.h"

OrdersPage::OrdersPage(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::OrdersPage)
{
    ui->setupUi(this);
}

OrdersPage::~OrdersPage()
{
    delete ui;
}
