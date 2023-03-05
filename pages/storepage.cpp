#include "storepage.h"
#include "ui_storepage.h"

StorePage::StorePage(QWidget *parent) :
    BasePage(parent),
    ui(new Ui::StorePage)
{
    ui->setupUi(this);
}

StorePage::~StorePage()
{
    delete ui;
}

void StorePage::setApiClient(api::GogApiClient *apiClient)
{

}

void StorePage::clear()
{
    ui->webView->scroll(0, 0);
}

void StorePage::initialize()
{

}
