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

void StorePage::setApiClient(GogApiClient *apiClient)
{

}

void StorePage::clear()
{

}

void StorePage::initialize()
{

}
