#include "orderspage.h"
#include "ui_orderspage.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QMenu>
#include <QNetworkReply>

#include "../api/utils/orderserialization.h"
#include "../widgets/ordergroup.h"

OrdersPage::OrdersPage(QWidget *parent) :
    StoreBasePage(Page::ORDER_HISTORY, parent),
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
        page = 1;
        fetchData();
    });

    item = filtersMenu->addAction("PENDING ORDERS");
    item->setCheckable(true);
    item->setChecked(true);
    connect(item, &QAction::toggled, this, [this](bool checked){
        filter.pending = checked;
        page = 1;
        fetchData();
    });

    item = filtersMenu->addAction("COMPLETED ORDERS");
    item->setCheckable(true);
    item->setChecked(true);
    connect(item, &QAction::toggled, this, [this](bool checked){
        filter.completed = checked;
        page = 1;
        fetchData();
    });

    item = filtersMenu->addAction("REFUNDED ORDERS");
    item->setCheckable(true);
    item->setChecked(false);
    connect(item, &QAction::toggled, this, [this](bool checked){
        filter.cancelled = checked;
        page = 1;
        fetchData();
    });

    item = filtersMenu->addAction("REDEEMED GIFTS");
    item->setCheckable(true);
    item->setChecked(true);
    connect(item, &QAction::toggled, this, [this](bool checked){
        filter.redeemed = checked;
        page = 1;
        fetchData();
    });

    item = filtersMenu->addAction("UNREDEEMED GIFTS");
    item->setCheckable(true);
    item->setChecked(true);
    connect(item, &QAction::toggled, this, [this](bool checked){
        filter.notRedeemed = checked;
        page = 1;
        fetchData();
    });

    ui->filtersToolButton->setMenu(filtersMenu);

    filter.inProgress = true;
    filter.pending = true;
    filter.completed = true;
    filter.cancelled = false;
    filter.redeemed = true;
    filter.notRedeemed = true;

    page = 1;
    paginator = new Pagination(ui->resultsPage);
    connect(paginator, &Pagination::changedPage, this, [this](quint16 newPage)
    {
       page = newPage;
       fetchData();
    });
    ui->resultsPageLayout->addWidget(paginator);
    ui->resultsPageLayout->setAlignment(paginator, Qt::AlignHCenter);
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
    ui->contentsStack->setCurrentWidget(ui->loaderPage);
    paginator->setVisible(false);
    while (!ui->resultsScrollContentsLayout->isEmpty())
    {
        auto item = ui->resultsScrollContentsLayout->itemAt(0);
        ui->resultsScrollContentsLayout->removeItem(item);
        delete item->widget();
        delete item;
    }

    auto networkReply = apiClient->getOrdersHistory(filter, page);
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
                for (const api::Order &item : std::as_const(data.orders))
                {
                    ui->resultsScrollContentsLayout->addWidget(new OrderGroup(item, apiClient, ui->resultsScrollContents));
                }
                paginator->changePages(page, data.totalPages);
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

void OrdersPage::initialize(const QVariant &data)
{
    fetchData();
}

void OrdersPage::switchUiAuthenticatedState(bool authenticated)
{
    StoreBasePage::switchUiAuthenticatedState(authenticated);
}

void OrdersPage::on_searchEdit_textChanged(const QString &arg1)
{
    page = 1;
    filter.query = arg1.trimmed();
    fetchData();
}

