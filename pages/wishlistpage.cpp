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
    StoreBasePage(Page::WISHLIST, parent),
    getWishlistReply(nullptr),
    setWishlistSharingReply(nullptr),
    ui(new Ui::WishlistPage)
{
    ui->setupUi(this);
    ui->resultsScrollAreaContentsLayout->setAlignment(Qt::AlignTop);
    connect(ui->searchEdit, &QLineEdit::textChanged, this, &WishlistPage::onSearchTextChanged);
    connect(ui->orderComboBox, &QComboBox::currentIndexChanged, this, &WishlistPage::onCurrentOrderChanged);
    connect(ui->visibilityComboBox, &QComboBox::currentIndexChanged, this, &WishlistPage::onCurrentVisibilityChanged);

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
    if (getWishlistReply != nullptr)
    {
        getWishlistReply->abort();
    }
    if (setWishlistSharingReply != nullptr)
    {
        setWishlistSharingReply->abort();
    }
    delete ui;
}

void WishlistPage::setApiClient(api::GogApiClient *apiClient)
{
    this->apiClient = apiClient;
}

void WishlistPage::fetchData()
{
    if (getWishlistReply != nullptr)
    {
        getWishlistReply->abort();
    }
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

    getWishlistReply = apiClient->getWishlist(query, orders[currentOrder], page);
    connect(getWishlistReply, &QNetworkReply::finished, this, [this]()
    {
        auto networkReply = getWishlistReply;
        getWishlistReply = nullptr;

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
                for (const api::Product &product : std::as_const(data.products))
                {
                    auto wishlistItem = new WishlistItem(product, apiClient, ui->resultsScrollAreaContents);
                    connect(wishlistItem, &WishlistItem::navigateToProduct, this, [this](quint64 productId)
                    {
                        emit navigate({Page::CATALOG_PRODUCT, productId});
                    });
                    ui->resultsScrollAreaContentsLayout->addWidget(wishlistItem);
                }
                ui->contentsStack->setCurrentWidget(ui->resultsPage);
                ui->titleLabel->setText(QString("WISHLISTED TITLES (%1)").arg(data.totalProducts));
                paginator->changePages(page, data.totalPages);
            }
            networkReply->deleteLater();
        }
        else if (networkReply->error() != QNetworkReply::OperationCanceledError)
        {
            ui->contentsStack->setCurrentWidget(ui->errorPage);
            qDebug() << networkReply->error() << networkReply->errorString() << QString(networkReply->readAll()).toUtf8();
        }

        networkReply->deleteLater();
    });
}

void WishlistPage::initialize(const QVariant &data)
{
    fetchData();
}

void WishlistPage::switchUiAuthenticatedState(bool authenticated)
{
    StoreBasePage::switchUiAuthenticatedState(authenticated);
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

void WishlistPage::onCurrentVisibilityChanged(int index)
{
    int wishlistVisibility;
    switch (index)
    {
    case 1:
        wishlistVisibility = 2;
        break;
    case 2:
        wishlistVisibility = 1;
        break;
    default:
        wishlistVisibility = 0;
    }

    setWishlistSharingReply = apiClient->setWishlistVisibility(wishlistVisibility);
    connect(setWishlistSharingReply, &QNetworkReply::finished, this, [this]()
    {
        auto networkReply = setWishlistSharingReply;
        setWishlistSharingReply = nullptr;
        if (networkReply->error() == QNetworkReply::NoError)
        {
            // TODO: show some kind of notification?
        }
        else if (networkReply->error() != QNetworkReply::OperationCanceledError)
        {
            qDebug() << networkReply->error() << networkReply->errorString() << QString(networkReply->readAll()).toUtf8();
        }

        networkReply->deleteLater();
    });
}

void WishlistPage::on_retryButton_clicked()
{
    fetchData();
}

