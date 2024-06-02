#include "storepage.h"
#include "ui_storepage.h"

#include <QNetworkReply>

StorePage::StorePage(QWidget *parent) :
    BasePage(parent),
    newsReply(nullptr),
    ui(new Ui::StorePage)
{
    ui->setupUi(this);
}

StorePage::~StorePage()
{
    if (newsReply != nullptr)
    {
        newsReply->abort();
    }
    delete ui;
}

void StorePage::setApiClient(api::GogApiClient *apiClient)
{

}

void StorePage::initialize(const QVariant &data)
{

}

void StorePage::switchUiAuthenticatedState(bool authenticated)
{

}

