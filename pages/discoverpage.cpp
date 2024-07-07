#include "discoverpage.h"
#include "ui_discoverpage.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkReply>

#include "../api/utils/newsserialization.h"
#include "../api/utils/storeserialization.h"
#include "../widgets/simpleproductitem.h"
#include "../widgets/newsitemtile.h"

DiscoverPage::DiscoverPage(QWidget *parent) :
    BasePage(parent),
    ui(new Ui::DiscoverPage)
{
    ui->setupUi(this);
}

DiscoverPage::~DiscoverPage()
{
    delete ui;
}

void DiscoverPage::setApiClient(api::GogApiClient *apiClient)
{
    this->apiClient = apiClient;
}

void DiscoverPage::getGoodOldGames()
{
    ui->customSectionGOGStackedWidget->setCurrentWidget(ui->customSectionGOGLoadingPage);
    auto customSectionGOGReply = apiClient->getStoreCustomSection("3a6bfab2-af69-11ec-9ed8-fa163ec3f57d");
    connect(customSectionGOGReply, &QNetworkReply::finished,
            this, [this, networkReply = customSectionGOGReply]()
    {
        if (networkReply->error() == QNetworkReply::NoError)
        {
            auto resultJson = QJsonDocument::fromJson(QString(networkReply->readAll()).toUtf8()).object();
            api::GetStoreCustomSectionResponse data;
            parseGetStoreCustomSectionResponse(resultJson, data, "_product_tile_256.webp");

            int column = 0;
            int row = 0;
            for (const api::StoreCustomSectionItem &item : std::as_const(data.items))
            {
                auto itemWidget = new SimpleProductItem(ui->customSectionGOGResultsPage);
                itemWidget->setCover(item.product.image, apiClient);
                itemWidget->setTitle(item.product.title);
                itemWidget->setPrice(item.product.price.baseAmount, item.product.price.finalAmount,
                                     item.product.price.discountPercentage, item.product.price.free, "");
                connect(this, &DiscoverPage::ownedProductsChanged,
                        itemWidget, [itemWidget, productId = item.product.id](const QSet<const QString> &ids)
                {
                    itemWidget->setOwned(ids.contains(productId));
                });
                itemWidget->setOwned(ownedProducts.contains(item.product.id));
                connect(this, &DiscoverPage::wishlistChanged,
                        itemWidget, [itemWidget, productId = item.product.id](const QSet<const QString> &ids)
                {
                    itemWidget->setWishlisted(ids.contains(productId));
                });
                itemWidget->setWishlisted(wishlist.contains(item.product.id));
                connect(apiClient, &api::GogApiClient::authenticated,
                        itemWidget, &SimpleProductItem::switchUiAuthenticatedState);
                itemWidget->switchUiAuthenticatedState(apiClient->isAuthenticated());
                connect(itemWidget, &SimpleProductItem::clicked,
                        this, [this, productId = item.product.id]()
                {
                    emit navigate({Page::CATALOG_PRODUCT, productId});
                });
                ui->customSectionGOGResultsScrollAreaContentsLayout->addWidget(itemWidget, row, column);
                column += row;
                row = (row + 1) % 2;
            }
            ui->customSectionGOGStackedWidget->setCurrentWidget(ui->customSectionGOGResultsPage);
        }
        else if (networkReply->error() != QNetworkReply::OperationCanceledError)
        {
            qDebug() << networkReply->error()
                     << networkReply->errorString()
                     << QString(networkReply->readAll()).toUtf8();
        }
        networkReply->deleteLater();
    });
    connect(this, &QObject::destroyed, customSectionGOGReply, &QNetworkReply::finished);
}

void DiscoverPage::getNews()
{
    ui->newsStackedWidget->setCurrentWidget(ui->newsLoadingPage);

    QString systemLanguage = QLocale::languageToCode(QLocale::system().language(), QLocale::ISO639Part1);
    auto newsReply = apiClient->getNews(0, systemLanguage, 11);
    connect(newsReply, &QNetworkReply::finished, this, [this, networkReply = newsReply]()
    {
        if (networkReply->error() == QNetworkReply::NoError)
        {
            auto resultJson = QJsonDocument::fromJson(QString(networkReply->readAll()).toUtf8()).object();
            api::GetNewsResponse data;
            parseNewsResponse(resultJson, data);

            bool primary = true;
            for (const api::NewsItem &item : std::as_const(data.items))
            {
                auto itemTile = new NewsItemTile(item, primary, apiClient, ui->newsScrollAreaContents);
                primary = false;
                connect(itemTile, &NewsItemTile::navigateToNewsItem, this,
                        [this](unsigned long long newsId)
                {
                    emit navigate({ Page::NEWS, newsId });
                });
                ui->newsScrollAreaContentsLayout->addWidget(itemTile);
            }
            ui->newsStackedWidget->setCurrentWidget(ui->newsResultsPage);
        }
        else if (networkReply->error() != QNetworkReply::OperationCanceledError)
        {
            qDebug() << networkReply->error()
                     << networkReply->errorString()
                     << QString(networkReply->readAll()).toUtf8();
        }
        networkReply->deleteLater();
    });
    connect(this, &QObject::destroyed, newsReply, &QNetworkReply::abort);
}

void DiscoverPage::getRecommendedDlc()
{
    ui->recommendedDlcStackedWidget->setCurrentWidget(ui->recommendedDlcLoadingPage);
    auto recommendedDlcReply = apiClient->getRecommendedDlcs();
    connect(recommendedDlcReply, &QNetworkReply::finished,
            this, [this, networkReply = recommendedDlcReply]()
    {
        if (networkReply->error() == QNetworkReply::NoError)
        {
            auto resultJson = QJsonDocument::fromJson(QString(networkReply->readAll()).toUtf8()).object();
            api::GetStoreRecommendedDlcsResponse data;
            parseGetStoreRecommendedDlcsResponse(resultJson, data);

            for (const api::StoreProduct &item : std::as_const(data.recommendations))
            {
                auto itemWidget = new SimpleProductItem(ui->recommendedDlcResultsPage);
                itemWidget->setCover(item.image, apiClient);
                itemWidget->setPrice(item.price.baseAmount, item.price.finalAmount, item.price.discountPercentage,
                                     item.price.free, "");
                itemWidget->setTitle(item.title);
                connect(this, &DiscoverPage::ownedProductsChanged,
                        itemWidget, [itemWidget, productId = item.id](const QSet<const QString> &ids)
                {
                    itemWidget->setOwned(ids.contains(productId));
                });
                itemWidget->setOwned(ownedProducts.contains(item.id));
                connect(this, &DiscoverPage::wishlistChanged,
                        itemWidget, [itemWidget, productId = item.id](const QSet<const QString> &ids)
                {
                    itemWidget->setWishlisted(ids.contains(productId));
                });
                itemWidget->setWishlisted(wishlist.contains(item.id));
                connect(apiClient, &api::GogApiClient::authenticated,
                        itemWidget, &SimpleProductItem::switchUiAuthenticatedState);
                itemWidget->switchUiAuthenticatedState(apiClient->isAuthenticated());
                connect(itemWidget, &SimpleProductItem::clicked,
                        this, [this, productId = item.id]()
                {
                    emit navigate({Page::CATALOG_PRODUCT, productId});
                });
                ui->recommendedDlcScrollAreaContentsLayout->addWidget(itemWidget);
            }
            ui->recommendedDlcScrollAreaContentsLayout->addStretch();
            ui->recommendedDlcStackedWidget->setCurrentWidget(ui->recommendedDlcResultsPage);

            ui->recommendedDlcLabel->setVisible(data.hasRecommendations);
            ui->recommendedDlcStackedWidget->setVisible(data.hasRecommendations);
        }
        else if (networkReply->error() != QNetworkReply::OperationCanceledError)
        {
            qDebug() << networkReply->error()
                     << networkReply->errorString()
                     << QString(networkReply->readAll()).toUtf8();
        }
        networkReply->deleteLater();
    });
    connect(this, &DiscoverPage::uiAuthenticatedSwitchRequested, recommendedDlcReply, &QNetworkReply::abort);
    connect(this, &QObject::destroyed, recommendedDlcReply, &QNetworkReply::abort);
}

void DiscoverPage::initialize(const QVariant &data)
{
    getGoodOldGames();
    getNews();
}

void DiscoverPage::switchUiAuthenticatedState(bool authenticated)
{
    emit uiAuthenticatedSwitchRequested();

    ui->recommendedDlcLabel->setVisible(false);
    ui->recommendedDlcStackedWidget->setVisible(false);

    QLayoutItem *item;
    while ((item = ui->recommendedDlcScrollAreaContentsLayout->takeAt(0)))
    {
        auto widget = item->widget();
        delete item;
        if (widget != nullptr)
        {
            delete widget;
        }
    }

    if (authenticated)
    {
        getRecommendedDlc();

        auto ownedProductsReply = apiClient->getOwnedLicensesIds();
        connect(ownedProductsReply, &QNetworkReply::finished,
                this, [this, networkReply = ownedProductsReply]()
        {
            if (networkReply->error() == QNetworkReply::NoError)
            {
                auto resultJson = QJsonDocument::fromJson(QString(networkReply->readAll()).toUtf8());
                auto ownedProducts = resultJson.toVariant().toList();
                for (const QVariant &id : std::as_const(ownedProducts))
                {
                    this->ownedProducts.insert(id.toString());
                }
                emit ownedProductsChanged(this->ownedProducts);
            }
            else if (networkReply->error() != QNetworkReply::OperationCanceledError)
            {
                qDebug() << networkReply->error()
                         << networkReply->errorString()
                         << QString(networkReply->readAll()).toUtf8();
            }

            networkReply->deleteLater();
        });
        connect(this, &DiscoverPage::uiAuthenticatedSwitchRequested, ownedProductsReply, &QNetworkReply::abort);
        connect(this, &QObject::destroyed, ownedProductsReply, &QNetworkReply::abort);

        auto wishlistReply = apiClient->getWishlistIds();
        connect(wishlistReply, &QNetworkReply::finished,
                this, [this, networkReply = wishlistReply]()
        {
            if (networkReply->error() == QNetworkReply::NoError)
            {
                auto resultJson = QJsonDocument::fromJson(QString(networkReply->readAll()).toUtf8());
                auto wishlistedItems = resultJson["wishlist"].toObject();
                for (const QString &key : wishlistedItems.keys())
                {
                    if (wishlistedItems[key].toBool())
                    {
                        wishlist.insert(key);
                    }
                }
                emit wishlistChanged(wishlist);
            }
            else if (networkReply->error() != QNetworkReply::OperationCanceledError)
            {
                qDebug() << networkReply->error()
                         << networkReply->errorString()
                         << QString(networkReply->readAll()).toUtf8();
            }

            networkReply->deleteLater();
        });
        connect(this, &DiscoverPage::uiAuthenticatedSwitchRequested, wishlistReply, &QNetworkReply::abort);
        connect(this, &QObject::destroyed, wishlistReply, &QNetworkReply::abort);
    }
    else
    {
        ownedProducts.clear();
        emit ownedProductsChanged(ownedProducts);
        wishlist.clear();
        emit wishlistChanged(wishlist);
    }
}
