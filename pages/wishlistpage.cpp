#include "wishlistpage.h"
#include "ui_wishlistpage.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QMenu>
#include <QMenuBar>
#include <QNetworkReply>

#include "../api/utils/serialization.h"
#include "../widgets/wishlistitem.h"

WishlistPage::WishlistPage(QWidget *parent) :
    BasePage(parent),
    ui(new Ui::WishlistPage)
{
    ui->setupUi(this);
    ui->resultsScrollAreaContentsLayout->setAlignment(Qt::AlignTop);
    connect(ui->searchEdit, &QLineEdit::textChanged, this, &WishlistPage::on_searchEdit_textChanged);
}

WishlistPage::~WishlistPage()
{
    delete ui;
}

void WishlistPage::setApiClient(api::GogApiClient *apiClient)
{
    this->apiClient = apiClient;
}

void WishlistPage::fetchData()
{
    clear();
    auto networkReply = apiClient->getWishlist(query);
    connect(networkReply, &QNetworkReply::finished, this, [=](){
        if (networkReply->error() == QNetworkReply::NoError)
        {
            auto resultJson = QJsonDocument::fromJson(QString(networkReply->readAll()).toUtf8()).object();
            api::GetWishlistResponse data;
            parseSearchWishlistResponse(resultJson, data);
            if (data.products.isEmpty())
            {
                ui->contentsStack->setCurrentWidget(ui->emptyPage);
                ui->titleLabel->setText("WISHLISTED TITLES");
            }
            else
            {
                api::Product product;
                foreach (product, data.products)
                {
                    ui->resultsScrollAreaContentsLayout->addWidget(new WishlistItem(product, apiClient, ui->resultsScrollAreaContents));
                }
                ui->contentsStack->setCurrentWidget(ui->resultsPage);
                ui->titleLabel->setText(QString("WISHLISTED TITLES (%1)").arg(data.totalProducts));
            }
            networkReply->deleteLater();
        }
    });
    connect(networkReply, &QNetworkReply::errorOccurred, this, [=](QNetworkReply::NetworkError error)
    {
        if (error != QNetworkReply::NoError)
        {
            networkReply->deleteLater();
        }
    });
}

void WishlistPage::clear()
{
    ui->contentsStack->setCurrentWidget(ui->loaderPage);
    while (!ui->resultsScrollAreaContentsLayout->isEmpty())
    {
        auto item =ui->resultsScrollAreaContentsLayout->itemAt(0);
        ui->resultsScrollAreaContentsLayout->removeItem(item);
        item->widget()->deleteLater();
        delete item;
    }
}

void WishlistPage::initialize()
{
    fetchData();
}

void WishlistPage::on_searchEdit_textChanged(const QString &arg1)
{
    query = arg1.trimmed();
    fetchData();
}

