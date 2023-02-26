#include "orderspage.h"
#include "ui_orderspage.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QMenu>
#include <QNetworkReply>

#include "../api/utils/orderserialization.h"
#include "../widgets/ordergroup.h"

OrdersPage::OrdersPage(QWidget *parent) :
    BasePage(parent),
    ui(new Ui::OrdersPage)
{
    ui->setupUi(this);
    ui->resultsScrollContentsLayout->setAlignment(Qt::AlignTop);

    auto filtersMenu = new QMenu(ui->filtersToolButton);

    QAction *item;

    item = filtersMenu->addAction("ORDERS IN PROGRESS");
    item->setCheckable(true);
    item->setChecked(true);
    connect(item, &QAction::toggled, this, [this](bool checked){
        filter.inProgress = checked;
        fetchData();
    });

    item = filtersMenu->addAction("PENDING ORDERS");
    item->setCheckable(true);
    item->setChecked(true);
    connect(item, &QAction::toggled, this, [this](bool checked){
        filter.pending = checked;
        fetchData();
    });

    item = filtersMenu->addAction("COMPLETED ORDERS");
    item->setCheckable(true);
    item->setChecked(true);
    connect(item, &QAction::toggled, this, [this](bool checked){
        filter.completed = checked;
        fetchData();
    });

    item = filtersMenu->addAction("REFUNDED ORDERS");
    item->setCheckable(true);
    item->setChecked(false);
    connect(item, &QAction::toggled, this, [this](bool checked){
        filter.cancelled = checked;
        fetchData();
    });

    item = filtersMenu->addAction("REDEEMED GIFTS");
    item->setCheckable(true);
    item->setChecked(true);
    connect(item, &QAction::toggled, this, [this](bool checked){
        filter.redeemed = checked;
        fetchData();
    });

    item = filtersMenu->addAction("UNREDEEMED GIFTS");
    item->setCheckable(true);
    item->setChecked(true);
    connect(item, &QAction::toggled, this, [this](bool checked){
        filter.notRedeemed = checked;
        fetchData();
    });

    ui->filtersToolButton->setMenu(filtersMenu);

    filter.inProgress = true;
    filter.pending = true;
    filter.completed = true;
    filter.cancelled = false;
    filter.redeemed = true;
    filter.notRedeemed = true;
}

OrdersPage::~OrdersPage()
{
    delete ui;
}

void OrdersPage::setApiClient(api::GogApiClient *apiClient)
{
    this->apiClient = apiClient;
}

void OrdersPage::fetchData()
{
    auto networkReply = apiClient->getOrdersHistory(filter);
    clear();
    connect(networkReply, &QNetworkReply::finished, this, [=](){
        if (networkReply->error() == QNetworkReply::NoError)
        {
            auto resultJson = QJsonDocument::fromJson(QString(networkReply->readAll()).toUtf8()).object();
            api::GetOrdersHistoryResponse data;
            parseGetOrdersHistoryResponse(resultJson, data);
            if (data.orders.isEmpty())
            {
                ui->contentsStack->setCurrentWidget(ui->emptyPage);
            }
            else
            {
                api::Order item;
                foreach (item, data.orders)
                {
                    ui->resultsScrollContentsLayout->addWidget(new OrderGroup(item, apiClient, ui->resultsScrollContents));
                }
                ui->contentsStack->setCurrentWidget(ui->resultsPage);
            }
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

void OrdersPage::clear()
{
    while (!ui->resultsScrollContentsLayout->isEmpty())
    {
        auto item = ui->resultsScrollContentsLayout->itemAt(0);
        ui->resultsScrollContentsLayout->removeItem(item);
        delete item->widget();
        delete item;
    }
    ui->contentsStack->setCurrentWidget(ui->loaderPage);
}

void OrdersPage::initialize()
{
    fetchData();
}

void OrdersPage::on_searchEdit_textChanged(const QString &arg1)
{
    filter.query = arg1.trimmed();
    fetchData();
}

