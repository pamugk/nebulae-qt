#include "wishlistpage.h"
#include "ui_wishlistpage.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QMenu>
#include <QNetworkReply>
#include <QScrollBar>

#include "../api/utils/serialization.h"
#include "../widgets/wishlistitem.h"

WishlistPage::WishlistPage(QWidget *parent) :
    BasePage(parent),
    ui(new Ui::WishlistPage)
{
    ui->setupUi(this);
    ui->resultsScrollAreaContentsLayout->setAlignment(Qt::AlignTop);
    connect(ui->searchEdit, &QLineEdit::textChanged, this, &WishlistPage::onSearchTextChanged);
    connect(ui->orderComboBox, &QComboBox::currentIndexChanged, this, &WishlistPage::onCurrentOrderChanged);

    orders = {"title", "date_added", "user_reviews"};
    currentOrder = 0;

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
    auto networkReply = apiClient->getWishlist(query, orders[currentOrder], page);
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
                foreach (api::Product product, data.products)
                {
                    auto wishlistItem = new WishlistItem(product, apiClient, ui->resultsScrollAreaContents);
                    connect(wishlistItem, &WishlistItem::navigateToProduct, this, [this](quint64 productId)
                    {
                        emit navigateToDestination({Page::CATALOG_PRODUCT_PAGE, productId});
                    });
                    ui->resultsScrollAreaContentsLayout->addWidget(wishlistItem);
                }
                ui->contentsStack->setCurrentWidget(ui->resultsPage);
                ui->titleLabel->setText(QString("WISHLISTED TITLES (%1)").arg(data.totalProducts));
                paginator->changePages(page, data.totalPages);
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
    ui->resultsScrollArea->verticalScrollBar()->setValue(0);
    paginator->setVisible(false);
    while (!ui->resultsScrollAreaContentsLayout->isEmpty())
    {
        auto item =ui->resultsScrollAreaContentsLayout->itemAt(0);
        ui->resultsScrollAreaContentsLayout->removeItem(item);
        item->widget()->deleteLater();
        delete item;
    }
}

void WishlistPage::initialize(const QVariant &data)
{
    fetchData();
}

void WishlistPage::onSearchTextChanged(const QString &arg1)
{
    page = 1;
    query = arg1.trimmed();
    fetchData();
}

void WishlistPage::onCurrentOrderChanged(int index)
{
    if (index < 0 || index > 2)
    {
        index = 0;
    }
    currentOrder = index;
    page = 1;
    fetchData();
}

