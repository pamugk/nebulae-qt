#include "orderspage.h"
#include "ui_orderspage.h"

OrdersPage::OrdersPage(QWidget *parent) :
    BasePage(parent),
    ui(new Ui::OrdersPage)
{
    ui->setupUi(this);
}

OrdersPage::~OrdersPage()
{
    delete ui;
}

void OrdersPage::setApiClient(GogApiClient *apiClient)
{

}

void OrdersPage::clear()
{

}

void OrdersPage::initialize()
{

}
