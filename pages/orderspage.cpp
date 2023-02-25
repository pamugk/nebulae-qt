#include "orderspage.h"
#include "ui_orderspage.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkReply>

#include "../api/utils/orderserialization.h"
#include "../widgets/ordergroup.h"

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
    this->apiClient = apiClient;
}

void OrdersPage::clear()
{
    while (!ui->resultsScrollContentsLayout->isEmpty())
    {
        ui->resultsScrollContentsLayout->removeItem(ui->resultsScrollContentsLayout->itemAt(0));
    }
    ui->contentsStack->setCurrentWidget(ui->loaderPage);
}

void OrdersPage::initialize()
{
    auto networkReply = apiClient->getOrdersHistory(true, true, true, true, true, true, QString());
    ui->contentsStack->setCurrentWidget(ui->loaderPage);
    connect(networkReply, &QNetworkReply::finished, this, [=](){
        if (networkReply->error() == QNetworkReply::NoError)
        {
            auto resultJson = QJsonDocument::fromJson(QString(networkReply->readAll()).toUtf8()).object();
            GetOrdersHistoryResponse data;
            parseGetOrdersHistoryResponse(resultJson, data);
            Order item;
            foreach (item, data.orders)
            {
                ui->resultsScrollContentsLayout->addWidget(new OrderGroup(item, apiClient, ui->resultsScrollContents));
            }
            ui->contentsStack->setCurrentWidget(ui->resultsPage);
            networkReply->deleteLater();
        }
    });
    connect(networkReply, &QNetworkReply::errorOccurred, this, [=](QNetworkReply::NetworkError error)
    {
        if (error != QNetworkReply::NoError)
        {
            qDebug() << error;
            networkReply->deleteLater();
        }
    });
}
