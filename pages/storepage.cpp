#include "storepage.h"
#include "ui_storepage.h"

StorePage::StorePage(QWidget *parent) :
    BasePage(parent),
    ui(new Ui::StorePage)
{
    ui->setupUi(this);
    connect(ui->webView, &QWebEngineView::loadProgress,
            ui->progressBar, &QProgressBar::setValue);
}

StorePage::~StorePage()
{
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

void StorePage::on_retryButton_clicked()
{
    ui->contentStack->setCurrentWidget(ui->loadingPage);
    ui->webView->reload();
}


void StorePage::on_webView_loadFinished(bool success)
{
    if (success)
    {
        ui->contentStack->setCurrentWidget(ui->mainContentPage);
    }
    else
    {
        ui->contentStack->setCurrentWidget(ui->errorPage);
    }
}

