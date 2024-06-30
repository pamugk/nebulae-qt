#include "discoverpage.h"
#include "ui_discoverpage.h"

DiscoverPage::DiscoverPage(QWidget *parent) :
    BasePage(parent),
    ui(new Ui::DiscoverPage)
{
    ui->setupUi(this);
}

DiscoverPage::~DiscoverPage()
{
    delete ui;
}

void DiscoverPage::setApiClient(api::GogApiClient *apiClient)
{
    this->apiClient = apiClient;
}

void DiscoverPage::initialize(const QVariant &data)
{

}

void DiscoverPage::switchUiAuthenticatedState(bool authenticated)
{

}
