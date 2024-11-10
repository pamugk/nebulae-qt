#include "librarypage.h"
#include "ui_librarypage.h"

#include <QDesktopServices>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkReply>
#include <QScrollBar>

#include "../api/utils/ownedproductserialization.h"
#include "../layouts/flowlayout.h"
#include "../widgets/ownedgogproductgridtile.h"
#include "../widgets/ownedgogproductlistitem.h"

LibraryPage::LibraryPage(QWidget *parent) :
    StoreBasePage(Page::GOG_LIBRARY, parent),
    gridLayout(true),
    ui(new Ui::LibraryPage)
{
    ui->setupUi(this);
    ui->resultsGridScrollAreaContents->setLayout(new FlowLayout(ui->resultsGridScrollAreaContents, -1, 20, 40));
    ui->resultsListLayout->setAlignment(Qt::AlignTop);

    page = 1;
    paginator = new Pagination(this);
    paginator->setVisible(false);
    connect(paginator, &Pagination::changedPage, this, [this](quint16 newPage)
    {
       page = newPage;
       fetchData();
    });
    ui->paginatorSlotLayout->addWidget(paginator, Qt::AlignHCenter);
}

LibraryPage::~LibraryPage()
{
    if (lastLibraryReply != nullptr)
    {
        lastLibraryReply->abort();
    }
    delete ui;
}

void LibraryPage::setApiClient(api::GogApiClient *apiClient)
{
    this->apiClient = apiClient;
}

void LibraryPage::fetchData()
{
    ui->contentsStack->setCurrentWidget(ui->loaderPage);
    data.products.clear();
    ui->resultsListScrollArea->verticalScrollBar()->setValue(0);
    while (!ui->resultsListLayout->isEmpty())
    {
        auto item = ui->resultsListLayout->itemAt(0);
        ui->resultsListLayout->removeItem(item);
        item->widget()->deleteLater();
        delete item;
    }
    ui->resultsGridScrollArea->verticalScrollBar()->setValue(0);
    while (!ui->resultsGridScrollAreaContents->layout()->isEmpty())
    {
        auto item = ui->resultsGridScrollAreaContents->layout()->itemAt(0);
        ui->resultsGridScrollAreaContents->layout()->removeItem(item);
        item->widget()->deleteLater();
        delete item;
    }

    if (lastLibraryReply != nullptr)
    {
        lastLibraryReply->abort();
    }

    lastLibraryReply = apiClient->getOwnedProducts();
    connect(lastLibraryReply, &QNetworkReply::finished, this, [this]()
    {
        auto networkReply = lastLibraryReply;
        lastLibraryReply = nullptr;
        if (networkReply->error() == QNetworkReply::NoError)
        {
            auto resultJson = QJsonDocument::fromJson(QString(networkReply->readAll()).toUtf8()).object();
            parseOwnedProductsResponse(resultJson, this->data);

            if (this->data.products.isEmpty())
            {
                paginator->setVisible(false);
                ui->contentsStack->setCurrentWidget(ui->emptyPage);
            }
            else
            {
                paginator->setVisible(true);
                paginator->changePages(page, this->data.totalPages);
                layoutResults();
            }
        }
        else if (networkReply->error() != QNetworkReply::OperationCanceledError)
        {   ui->contentsStack->setCurrentWidget(ui->errorPage);
            qDebug() << networkReply->error() << networkReply->errorString() << QString(networkReply->readAll()).toUtf8();
        }

        networkReply->deleteLater();
    });
}

void LibraryPage::layoutResults()
{
    ui->contentsStack->setCurrentWidget(ui->loaderPage);
    if (gridLayout)
    {
        for (const api::OwnedProduct &product : std::as_const(data.products))
        {
            auto storeItem = new OwnedGogProductGridTile(ui->resultsListPage);
            auto coverReply = apiClient->getAnything(QLatin1StringView("https:%1_196.jpg").arg(product.image));
            connect(coverReply, &QNetworkReply::finished, storeItem, [storeItem, coverReply]() {
                if (coverReply->error() == QNetworkReply::NoError)
                {
                    QPixmap image;
                    image.loadFromData(coverReply->readAll());
                    storeItem->setCover(image);
                }
                coverReply->deleteLater();
            });
            connect(storeItem, &QObject::destroyed, coverReply, &QNetworkReply::abort);
            storeItem->setTitle(product.title);

            connect(storeItem, &OwnedGogProductGridTile::clicked, this, [this, productId = product.id]()
            {
            });
            connect(storeItem, &OwnedGogProductGridTile::requestedInstallation, this, [this, productId = product.id]()
            {
            });
            connect(storeItem, &OwnedGogProductGridTile::requestedSettings, this, [this, productId = product.id]()
            {
            });
            connect(storeItem, &OwnedGogProductGridTile::requestedDownloads, this, [this, productId = product.id]()
            {
            });
            connect(storeItem, &OwnedGogProductGridTile::requestedStore, this, [this, productId = product.id]()
            {
            });
            connect(storeItem, &OwnedGogProductGridTile::requestedSupport, this, &LibraryPage::openSupport);
            connect(storeItem, &OwnedGogProductGridTile::requestedToHide, this, [this, productId = product.id]()
            {
            });

            ui->resultsGridScrollAreaContents->layout()->addWidget(storeItem);
        }
        ui->resultsStackedWidget->setCurrentWidget(ui->resultsGridPage);
    }
    else
    {
        for (const api::OwnedProduct &product : std::as_const(data.products))
        {
            auto storeItem = new OwnedGogProductListItem(ui->resultsListPage);
            auto coverReply = apiClient->getAnything(QLatin1StringView("https:%1_196.jpg").arg(product.image));
            connect(coverReply, &QNetworkReply::finished, storeItem, [storeItem, coverReply]() {
                if (coverReply->error() == QNetworkReply::NoError)
                {
                    QPixmap image;
                    image.loadFromData(coverReply->readAll());
                    storeItem->setCover(image);
                }
                coverReply->deleteLater();
            });
            connect(storeItem, &QObject::destroyed, coverReply, &QNetworkReply::abort);
            storeItem->setTitle(product.title);
            connect(storeItem, &OwnedGogProductListItem::clicked, this, [this, productId = product.id]()
            {
            });
            ui->resultsListLayout->addWidget(storeItem);
        }
        ui->resultsStackedWidget->setCurrentWidget(ui->resultsListPage);
    }
    ui->contentsStack->setCurrentWidget(ui->resultsPage);
}

void LibraryPage::initialize(const QVariant &data)
{
    lastLibraryReply = apiClient->getOwnedProducts();
    ui->contentsStack->setCurrentWidget(ui->loaderPage);
    connect(lastLibraryReply, &QNetworkReply::finished, this, [this]()
    {
        auto networkReply = lastLibraryReply;
        lastLibraryReply = nullptr;
        if (networkReply->error() == QNetworkReply::NoError)
        {
            auto resultJson = QJsonDocument::fromJson(QString(networkReply->readAll()).toUtf8()).object();
            parseOwnedProductsResponse(resultJson, this->data);

            if (this->data.products.isEmpty())
            {
                paginator->setVisible(false);
                ui->contentsStack->setCurrentWidget(ui->emptyPage);
            }
            else
            {
                paginator->setVisible(true);
                paginator->changePages(page, this->data.totalPages);
                layoutResults();
            }
        }
        else if (networkReply->error() != QNetworkReply::OperationCanceledError)
        {   ui->contentsStack->setCurrentWidget(ui->errorPage);
            qDebug() << networkReply->error() << networkReply->errorString() << QString(networkReply->readAll()).toUtf8();
        }

        networkReply->deleteLater();
    });
}

void LibraryPage::switchUiAuthenticatedState(bool authenticated)
{
    StoreBasePage::switchUiAuthenticatedState(authenticated);
}

void LibraryPage::openSupport()
{
    QDesktopServices::openUrl(QUrl("https://support.gog.com/hc?product=galaxy"));
}
