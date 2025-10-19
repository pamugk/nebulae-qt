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
    delete ui;
}

void WishlistPage::setApiClient(api::GogApiClient *apiClient)
{
    this->apiClient = apiClient;
}

void WishlistPage::fetchData()
{
    emit updatingData();
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

    QNetworkReply *wishlistReply = apiClient->getWishlist(query, orders[currentOrder], page);
    connect(this, &QObject::destroyed, wishlistReply, &QNetworkReply::abort);
    connect(this, &WishlistPage::updatingData, wishlistReply, &QNetworkReply::abort);
    connect(wishlistReply, &QNetworkReply::finished, this, [this, wishlistReply]()
    {
        if (wishlistReply->error() == QNetworkReply::NoError)
        {
            auto resultJson = QJsonDocument::fromJson(QString(wishlistReply->readAll()).toUtf8()).object();
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
                    connect(wishlistItem, &WishlistItem::clicked, this, [this, productId = product.id]()
                    {
                        emit navigate({Page::CATALOG_PRODUCT, productId});
                    });
                    ui->resultsScrollAreaContentsLayout->addWidget(wishlistItem);
                }
                ui->contentsStack->setCurrentWidget(ui->resultsPage);
                ui->titleLabel->setText(QString("WISHLISTED TITLES (%1)").arg(data.totalProducts));
                paginator->changePages(page, data.totalPages);
            }
        }
        else if (wishlistReply->error() != QNetworkReply::OperationCanceledError)
        {
            ui->contentsStack->setCurrentWidget(ui->errorPage);
            qDebug() << wishlistReply->error() << wishlistReply->errorString() << QString(wishlistReply->readAll()).toUtf8();
        }
    });
    connect(wishlistReply, &QNetworkReply::finished, wishlistReply, &QNetworkReply::deleteLater);
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

    QNetworkReply *setWishlistSharingReply = apiClient->setWishlistVisibility(wishlistVisibility);
    connect(this, &QObject::destroyed, setWishlistSharingReply, &QNetworkReply::abort);
    connect(setWishlistSharingReply, &QNetworkReply::finished, this, [this, setWishlistSharingReply]()
    {
        if (setWishlistSharingReply->error() == QNetworkReply::NoError)
        {
            // TODO: show some kind of notification?
        }
        else if (setWishlistSharingReply->error() != QNetworkReply::OperationCanceledError)
        {
            qDebug() << setWishlistSharingReply->error() << setWishlistSharingReply->errorString() << QString(setWishlistSharingReply->readAll()).toUtf8();
        }

    });
    connect(setWishlistSharingReply, &QNetworkReply::finished, setWishlistSharingReply, &QNetworkReply::deleteLater);
}

void WishlistPage::on_retryButton_clicked()
{
    fetchData();
}

