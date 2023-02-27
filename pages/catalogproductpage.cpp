#include "catalogproductpage.h"
#include "ui_catalogproductpage.h"

#include <QJsonDocument>
#include <QNetworkReply>

#include "../api/utils/catalogproductserialization.h"

CatalogProductPage::CatalogProductPage(QWidget *parent) :
    BasePage(parent),
    ui(new Ui::CatalogProductPage)
{
    ui->setupUi(this);
}

CatalogProductPage::~CatalogProductPage()
{
    delete ui;
}

void CatalogProductPage::setApiClient(api::GogApiClient *apiClient)
{
    this->apiClient = apiClient;
}

void CatalogProductPage::setProductId(quint64 id)
{
    this->id = id;
}

void CatalogProductPage::clear()
{
    ui->descriptionView->setUrl(QUrl("about:blank"));
    id = 0;
}

void CatalogProductPage::initialize()
{
    auto networkReply = apiClient->getCatalogProductInfo(id, "en-US");
    connect(networkReply, &QNetworkReply::finished, this, [this, networkReply]()
    {
        if (networkReply->error() == QNetworkReply::NoError)
        {
            auto resultJson = QJsonDocument::fromJson(QString(networkReply->readAll()).toUtf8()).object();
            api::GetCatalogProductInfoResponse data;
            parseCatalogProductInfoResponse(resultJson, data);

            ui->descriptionView->setHtml(data.description);
            ui->contentStack->setCurrentWidget(ui->mainPage);

            networkReply->deleteLater();
        }
    });
    connect(networkReply, &QNetworkReply::errorOccurred, this, [this, networkReply](QNetworkReply::NetworkError error)
    {
        if (error != QNetworkReply::NoError)
        {
            networkReply->deleteLater();
        }
    });
}

void CatalogProductPage::on_acceptContentWarningButton_clicked()
{
    ui->contentStack->setCurrentWidget(ui->mainPage);
}


void CatalogProductPage::on_goBackButton_clicked()
{
    emit navigateToDestination({Page::ALL_GAMES, QVariant()});
}

