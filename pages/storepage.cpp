#include "storepage.h"
#include "ui_storepage.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkReply>

#include "../api/utils/catalogserialization.h"
#include "../api/utils/newsserialization.h"
#include "../api/utils/storeserialization.h"
#include "../widgets/simpleproductitem.h"
#include "../widgets/storediscoveritem.h"
#include "../widgets/storesalebrowseallcard.h"
#include "../widgets/storesalecard.h"
#include "../widgets/newsitemtile.h"

StorePage::StorePage(QWidget *parent) :
    StoreBasePage(Page::STORE, parent),
    apiClient(nullptr),
    customSectionCDPRReply(nullptr),
    customSectionExclusivesReply(nullptr),
    customSectionGOGReply(nullptr),
    dealOfTheDayReply(nullptr),
    discoverBestsellingReply(nullptr),
    discoverGamesForYouReply(nullptr),
    discoverNewReply(nullptr),
    discoverUpcomingReply(nullptr),
    newsReply(nullptr),
    nowOnSaleReply(nullptr),
    ownedProductsReply(nullptr),
    recommendedDlcReply(nullptr),
    wishlistReply(nullptr),
    ui(new Ui::StorePage)
{
    ui->setupUi(this);
}

StorePage::~StorePage()
{
    if (customSectionCDPRReply != nullptr)
    {
        customSectionCDPRReply->abort();
    }
    if (customSectionExclusivesReply != nullptr)
    {
        customSectionExclusivesReply->abort();
    }
    if (customSectionGOGReply != nullptr)
    {
        customSectionGOGReply->abort();
    }
    if (dealOfTheDayReply != nullptr)
    {
        dealOfTheDayReply->abort();
    }
    if (discoverBestsellingReply != nullptr)
    {
        discoverBestsellingReply->abort();
    }
    if (discoverGamesForYouReply != nullptr)
    {
        discoverGamesForYouReply->abort();
    }
    if (discoverNewReply != nullptr)
    {
        discoverNewReply->abort();
    }
    if (discoverUpcomingReply != nullptr)
    {
        discoverUpcomingReply->abort();
    }
    if (newsReply != nullptr)
    {
        newsReply->abort();
    }
    if (nowOnSaleReply != nullptr)
    {
        nowOnSaleReply->abort();
    }
    for (QNetworkReply *nowOnSaleSectionReply : std::as_const(nowOnSaleSectionReplies))
    {
        if (nowOnSaleSectionReply != nullptr)
        {
            nowOnSaleSectionReply->abort();
        }
    }
    if (ownedProductsReply != nullptr)
    {
        ownedProductsReply->abort();
    }
    if (recommendedDlcReply != nullptr)
    {
        recommendedDlcReply->abort();
    }
    if (wishlistReply != nullptr)
    {
        wishlistReply->abort();
    }
    delete ui;
}

void StorePage::setApiClient(api::GogApiClient *apiClient)
{
    this->apiClient = apiClient;
}

void StorePage::getCustomSectionCDPRGames()
{
    ui->customSectionCDPRStackedWidget->setCurrentWidget(ui->customSectionCDPRLoadingPage);
    customSectionCDPRReply = apiClient->getStoreCustomSection("68469ed0-e0d6-11ec-a381-fa163eebc216");
    connect(customSectionCDPRReply, &QNetworkReply::finished,
            this, [this]()
    {
        auto networkReply = customSectionCDPRReply;
        customSectionCDPRReply = nullptr;

        if (networkReply->error() == QNetworkReply::NoError)
        {
            auto resultJson = QJsonDocument::fromJson(QString(networkReply->readAll()).toUtf8()).object();
            api::GetStoreCustomSectionResponse data;
            parseGetStoreCustomSectionResponse(resultJson, data, "_product_tile_256.webp");

            for (const api::StoreCustomSectionItem &item : std::as_const(data.items))
            {
                auto itemWidget = new SimpleProductItem(ui->customSectionCDPRScrollAreaContents);
                itemWidget->setCover(item.product.image, apiClient);
                itemWidget->setTitle(item.product.title);
                itemWidget->setPrice(item.product.price.baseAmount, item.product.price.finalAmount,
                                     item.product.price.discountPercentage, item.product.price.free, "");
                connect(this, &StorePage::ownedProductsChanged,
                        itemWidget, [itemWidget, productId = item.product.id](const QSet<const QString> &ids)
                {
                    itemWidget->setOwned(ids.contains(productId));
                });
                itemWidget->setOwned(ownedProducts.contains(item.product.id));
                connect(this, &StorePage::wishlistChanged,
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
                ui->customSectionCDPRScrollAreaContentsLayout->addWidget(itemWidget);
            }
            ui->customSectionCDPRScrollAreaContentsLayout->addStretch();
            ui->customSectionCDPRStackedWidget->setCurrentWidget(ui->customSectionCDPRResultsPage);
        }
        else if (networkReply->error() != QNetworkReply::OperationCanceledError)
        {
            qDebug() << networkReply->error()
                     << networkReply->errorString()
                     << QString(networkReply->readAll()).toUtf8();
        }
        networkReply->deleteLater();
    });
}

void StorePage::getCustomSectionExclusiveGames()
{
    ui->customSectionExclusivesStackedWidget->setCurrentWidget(ui->customSectionExclusivesLoadingPage);
    customSectionExclusivesReply = apiClient->getStoreCustomSection("eea11712-458e-11ee-9787-fa163eebc216");
    connect(customSectionExclusivesReply, &QNetworkReply::finished,
            this, [this]()
    {
        auto networkReply = customSectionExclusivesReply;
        customSectionExclusivesReply = nullptr;

        if (networkReply->error() == QNetworkReply::NoError)
        {
            auto resultJson = QJsonDocument::fromJson(QString(networkReply->readAll()).toUtf8()).object();
            api::GetStoreCustomSectionResponse data;
            parseGetStoreCustomSectionResponse(resultJson, data, "_product_tile_349.webp");

            int column = 0;
            int row = 0;
            for (const api::StoreCustomSectionItem &item : std::as_const(data.items))
            {
                auto itemWidget = new SimpleProductItem(ui->customSectionExclusivesResultsScrollAreaContents);
                itemWidget->setCover(item.product.image, apiClient);
                itemWidget->setTitle(item.product.title);
                itemWidget->setPrice(item.product.price.baseAmount, item.product.price.finalAmount,
                                     item.product.price.discountPercentage, item.product.price.free, "");
                connect(this, &StorePage::ownedProductsChanged,
                        itemWidget, [itemWidget, productId = item.product.id](const QSet<const QString> &ids)
                {
                    itemWidget->setOwned(ids.contains(productId));
                });
                itemWidget->setOwned(ownedProducts.contains(item.product.id));
                connect(this, &StorePage::wishlistChanged,
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
                ui->customSectionExclusivesResultsScrollAreaContentsLayout->addWidget(itemWidget, row, column);
                column += row;
                row = (row + 1) % 2;
            }
            ui->customSectionExclusivesStackedWidget->setCurrentWidget(ui->customSectionExclusivesResultsPage);
        }
        else if (networkReply->error() != QNetworkReply::OperationCanceledError)
        {
            qDebug() << networkReply->error()
                     << networkReply->errorString()
                     << QString(networkReply->readAll()).toUtf8();
        }
        networkReply->deleteLater();
    });
}

void StorePage::getCustomSectionGOGGames()
{
    ui->customSectionGOGStackedWidget->setCurrentWidget(ui->customSectionGOGLoadingPage);
    customSectionGOGReply = apiClient->getStoreCustomSection("3a6bfab2-af69-11ec-9ed8-fa163ec3f57d");
    connect(customSectionGOGReply, &QNetworkReply::finished,
            this, [this]()
    {
        auto networkReply = customSectionGOGReply;
        customSectionGOGReply = nullptr;

        if (networkReply->error() == QNetworkReply::NoError)
        {
            auto resultJson = QJsonDocument::fromJson(QString(networkReply->readAll()).toUtf8()).object();
            api::GetStoreCustomSectionResponse data;
            parseGetStoreCustomSectionResponse(resultJson, data, "_product_tile_256.webp");

            int column = 0;
            int row = 0;
            for (const api::StoreCustomSectionItem &item : std::as_const(data.items))
            {
                auto itemWidget = new SimpleProductItem(ui->customSectionCDPRScrollAreaContents);
                itemWidget->setCover(item.product.image, apiClient);
                itemWidget->setTitle(item.product.title);
                itemWidget->setPrice(item.product.price.baseAmount, item.product.price.finalAmount,
                                     item.product.price.discountPercentage, item.product.price.free, "");
                connect(this, &StorePage::ownedProductsChanged,
                        itemWidget, [itemWidget, productId = item.product.id](const QSet<const QString> &ids)
                {
                    itemWidget->setOwned(ids.contains(productId));
                });
                itemWidget->setOwned(ownedProducts.contains(item.product.id));
                connect(this, &StorePage::wishlistChanged,
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
}

void StorePage::getDealOfTheDay()
{
    ui->dealOfTheDayLabel->setVisible(false);
    ui->dealOfTheDayScrollArea->setVisible(false);
    dealOfTheDayReply = apiClient->getStoreCustomSection("bd39ffdc-458d-11ee-a513-fa163eebc216");
    connect(dealOfTheDayReply, &QNetworkReply::finished,
            this, [this]()
    {
        auto networkReply = dealOfTheDayReply;
        dealOfTheDayReply = nullptr;

        if (networkReply->error() == QNetworkReply::NoError)
        {
            auto resultJson = QJsonDocument::fromJson(QString(networkReply->readAll()).toUtf8()).object();
            api::GetStoreCustomSectionResponse data;
            parseGetStoreCustomSectionResponse(resultJson, data, "_product_tile_256.webp");

            QDateTime now = QDateTime::currentDateTime();
            // Also hide this section when the time comes?
            if (data.visibleFrom <= now && data.visibleTo >= now)
            {
                for (const api::StoreCustomSectionItem &item : std::as_const(data.items))
                {
                    auto itemWidget = new SimpleProductItem(ui->dealOfTheDayScrollAreaContents);
                    itemWidget->setCover(item.product.image, apiClient);
                    itemWidget->setTitle(item.product.title);
                    itemWidget->setDeal(item.dealActiveTo);
                    itemWidget->setPrice(item.product.price.baseAmount, item.product.price.finalAmount,
                                         item.product.price.discountPercentage, item.product.price.free, "");
                    connect(this, &StorePage::ownedProductsChanged,
                            itemWidget, [itemWidget, productId = item.product.id](const QSet<const QString> &ids)
                    {
                        itemWidget->setOwned(ids.contains(productId));
                    });
                    itemWidget->setOwned(ownedProducts.contains(item.product.id));
                    connect(this, &StorePage::wishlistChanged,
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
                    ui->dealOfTheDayScrollAreaContentsLayout->addWidget(itemWidget);
                }
                ui->dealOfTheDayScrollAreaContentsLayout->addStretch();
                ui->dealOfTheDayLabel->setVisible(!data.items.isEmpty());
                ui->dealOfTheDayScrollArea->setVisible(!data.items.isEmpty());
            }
            else
            {
                ui->dealOfTheDayLabel->setVisible(false);
                ui->dealOfTheDayScrollArea->setVisible(false);
            }
        }
        else if (networkReply->error() != QNetworkReply::OperationCanceledError)
        {
            qDebug() << networkReply->error()
                     << networkReply->errorString()
                     << QString(networkReply->readAll()).toUtf8();
        }
        networkReply->deleteLater();
    });
}

void StorePage::getDiscoverBestsellingGames()
{
    auto systemLocale = QLocale::system();
    ui->discoverBestsellingStackedWidget->setCurrentWidget(ui->discoverBestsellingLoadingPage);
    discoverBestsellingReply = apiClient->searchCatalog({ "trending", false }, {},
                                                        QLocale::territoryToCode(systemLocale.territory()),
                                                        systemLocale.name(QLocale::TagSeparator::Dash),
                                                        systemLocale.currencySymbol(QLocale::CurrencyIsoCode), 1, 8);
    connect(discoverBestsellingReply, &QNetworkReply::finished,
            this, [this]() {
        auto networkReply = discoverBestsellingReply;
        discoverBestsellingReply = nullptr;

        if (networkReply->error() == QNetworkReply::NoError)
        {
            auto resultJson = QJsonDocument::fromJson(QString(networkReply->readAll()).toUtf8()).object();
            api::SearchCatalogResponse data;
            parseSearchCatalogResponse(resultJson, data);

            for (const api::CatalogProduct &item : std::as_const(data.products))
            {
                auto itemWidget = new StoreDiscoverItem(ui->discoverBestsellingResultsPage);
                itemWidget->setCover(item.coverHorizontal, apiClient);
                itemWidget->setTitle(item.title);
                if (item.price.has_value())
                {
                    const auto &price = item.price.value();
                    itemWidget->setPrice(price.baseMoney.amount, price.finalMoney.amount,
                                         price.discount.isNull() || price.baseMoney.amount == 0
                                            ? 0
                                            : std::round(100. * (price.baseMoney.amount - price.finalMoney.amount) / price.baseMoney.amount),
                                         price.finalMoney.amount == 0, price.finalMoney.currency);
                }
                connect(this, &StorePage::ownedProductsChanged,
                        itemWidget, [itemWidget, productId = item.id](const QSet<const QString> &ids)
                {
                    itemWidget->setOwned(ids.contains(productId));
                });
                itemWidget->setOwned(ownedProducts.contains(item.id));
                connect(this, &StorePage::wishlistChanged,
                        itemWidget, [itemWidget, productId = item.id](const QSet<const QString> &ids)
                {
                    itemWidget->setWishlisted(ids.contains(productId));
                });
                itemWidget->setWishlisted(wishlist.contains(item.id));
                connect(apiClient, &api::GogApiClient::authenticated,
                        itemWidget, &StoreDiscoverItem::switchUiAuthenticatedState);
                itemWidget->switchUiAuthenticatedState(apiClient->isAuthenticated());
                connect(itemWidget, &StoreDiscoverItem::clicked,
                        this, [this, productId = item.id]()
                {
                    emit navigate({Page::CATALOG_PRODUCT, productId});
                });
                ui->discoverBestsellingResultsPageLayout->addWidget(itemWidget);
            }
            ui->discoverBestsellingResultsPageLayout->addStretch();
            ui->discoverBestsellingStackedWidget->setCurrentWidget(ui->discoverBestsellingResultsPage);
        }
        else if (networkReply->error() != QNetworkReply::OperationCanceledError)
        {
            qDebug() << networkReply->error()
                     << networkReply->errorString()
                     << QString(networkReply->readAll()).toUtf8();
        }
        networkReply->deleteLater();
    });
}

void StorePage::getDiscoverGamesForYou()
{
    ui->discoverGamesForYouStackedWidget->setCurrentWidget(ui->discoverGamesForYouLoadingPage);
    discoverGamesForYouReply = apiClient->getStoreDiscoverGamesForYou();
    connect(discoverGamesForYouReply, &QNetworkReply::finished,
            this, [this]() {
        auto networkReply = discoverGamesForYouReply;
        discoverGamesForYouReply = nullptr;

        if (networkReply->error() == QNetworkReply::NoError)
        {
            auto resultJson = QJsonDocument::fromJson(QString(networkReply->readAll()).toUtf8()).object();
            api::GetStoreDiscoverGamesSectionResponse data;
            parseGetStoreDiscoverGamesResponse(resultJson, data);

            for (const api::StoreProduct &item : std::as_const(data.personalizedProducts))
            {
                auto itemWidget = new StoreDiscoverItem(ui->discoverGamesForYouResultsPage);
                itemWidget->setCover(item.image, apiClient);
                itemWidget->setTitle(item.title);
                itemWidget->setPrice(item.price.baseAmount, item.price.finalAmount, item.price.discountPercentage,
                                     item.price.free, "");
                connect(this, &StorePage::ownedProductsChanged,
                        itemWidget, [itemWidget, productId = item.id](const QSet<const QString> &ids)
                {
                    itemWidget->setOwned(ids.contains(productId));
                });
                itemWidget->setOwned(ownedProducts.contains(item.id));
                connect(this, &StorePage::wishlistChanged,
                        itemWidget, [itemWidget, productId = item.id](const QSet<const QString> &ids)
                {
                    itemWidget->setWishlisted(ids.contains(productId));
                });
                itemWidget->setWishlisted(wishlist.contains(item.id));
                connect(apiClient, &api::GogApiClient::authenticated,
                        itemWidget, &StoreDiscoverItem::switchUiAuthenticatedState);
                itemWidget->switchUiAuthenticatedState(apiClient->isAuthenticated());
                connect(itemWidget, &StoreDiscoverItem::clicked,
                        this, [this, productId = item.id]()
                {
                    emit navigate({Page::CATALOG_PRODUCT, productId});
                });
                ui->discoverGamesForYouResultsPageLayout->addWidget(itemWidget);
            }
            ui->discoverGamesForYouResultsPageLayout->addStretch();
            ui->discoverGamesForYouStackedWidget->setCurrentWidget(ui->discoverGamesForYouResultsPage);
        }
        else if (networkReply->error() != QNetworkReply::OperationCanceledError)
        {
            qDebug() << networkReply->error()
                     << networkReply->errorString()
                     << QString(networkReply->readAll()).toUtf8();
        }
        networkReply->deleteLater();
    });
}

void StorePage::getDiscoverNewGames()
{
    ui->discoverNewStackedWidget->setCurrentWidget(ui->discoverNewLoadingPage);
    discoverNewReply = apiClient->getStoreDiscoverNewGames();
    connect(discoverNewReply, &QNetworkReply::finished,
            this, [this]() {
        auto networkReply = discoverNewReply;
        discoverNewReply = nullptr;

        if (networkReply->error() == QNetworkReply::NoError)
        {
            auto resultJson = QJsonDocument::fromJson(QString(networkReply->readAll()).toUtf8()).object();
            api::GetStoreDiscoverGamesSectionResponse data;
            parseGetStoreDiscoverGamesResponse(resultJson, data);

            for (const api::StoreProduct &item : std::as_const(data.personalizedProducts))
            {
                auto itemWidget = new StoreDiscoverItem(ui->discoverNewResultsPage);
                itemWidget->setCover(item.image, apiClient);
                itemWidget->setTitle(item.title);
                itemWidget->setPrice(item.price.baseAmount, item.price.finalAmount, item.price.discountPercentage,
                                     item.price.free, "");
                connect(this, &StorePage::ownedProductsChanged,
                        itemWidget, [itemWidget, productId = item.id](const QSet<const QString> &ids)
                {
                    itemWidget->setOwned(ids.contains(productId));
                });
                itemWidget->setOwned(ownedProducts.contains(item.id));
                connect(this, &StorePage::wishlistChanged,
                        itemWidget, [itemWidget, productId = item.id](const QSet<const QString> &ids)
                {
                    itemWidget->setWishlisted(ids.contains(productId));
                });
                itemWidget->setWishlisted(wishlist.contains(item.id));
                connect(apiClient, &api::GogApiClient::authenticated,
                        itemWidget, &StoreDiscoverItem::switchUiAuthenticatedState);
                itemWidget->switchUiAuthenticatedState(apiClient->isAuthenticated());
                connect(itemWidget, &StoreDiscoverItem::clicked,
                        this, [this, productId = item.id]()
                {
                    emit navigate({Page::CATALOG_PRODUCT, productId});
                });
                ui->discoverNewResultsPageLayout->addWidget(itemWidget);
            }
            ui->discoverNewResultsPageLayout->addStretch();
            ui->discoverNewStackedWidget->setCurrentWidget(ui->discoverNewResultsPage);
        }
        else if (networkReply->error() != QNetworkReply::OperationCanceledError)
        {
            qDebug() << networkReply->error()
                     << networkReply->errorString()
                     << QString(networkReply->readAll()).toUtf8();
        }
        networkReply->deleteLater();
    });
}

void StorePage::getDiscoverUpcomingGames()
{
    ui->discoverUpcomingStackedWidget->setCurrentWidget(ui->discoverUpcomingLoadingPage);
    discoverUpcomingReply = apiClient->getStoreDiscoverUpcomingGames();
    connect(discoverUpcomingReply, &QNetworkReply::finished,
            this, [this]() {
        auto networkReply = discoverUpcomingReply;
        discoverUpcomingReply = nullptr;

        if (networkReply->error() == QNetworkReply::NoError)
        {
            auto resultJson = QJsonDocument::fromJson(QString(networkReply->readAll()).toUtf8()).object();
            api::GetStoreDiscoverGamesSectionResponse data;
            parseGetStoreDiscoverGamesResponse(resultJson, data);

            for (const api::StoreProduct &item : std::as_const(data.personalizedProducts))
            {
                auto itemWidget = new StoreDiscoverItem(ui->discoverUpcomingResultsPage);
                itemWidget->setCover(item.image, apiClient);
                itemWidget->setPreorder(item.preorder);
                itemWidget->setPrice(item.price.baseAmount, item.price.finalAmount, item.price.discountPercentage,
                                     item.price.free, "");
                itemWidget->setTitle(item.title);
                connect(this, &StorePage::ownedProductsChanged,
                        itemWidget, [itemWidget, productId = item.id](const QSet<const QString> &ids)
                {
                    itemWidget->setOwned(ids.contains(productId));
                });
                itemWidget->setOwned(ownedProducts.contains(item.id));
                connect(this, &StorePage::wishlistChanged,
                        itemWidget, [itemWidget, productId = item.id](const QSet<const QString> &ids)
                {
                    itemWidget->setWishlisted(ids.contains(productId));
                });
                itemWidget->setWishlisted(wishlist.contains(item.id));
                connect(apiClient, &api::GogApiClient::authenticated,
                        itemWidget, &StoreDiscoverItem::switchUiAuthenticatedState);
                itemWidget->switchUiAuthenticatedState(apiClient->isAuthenticated());
                connect(itemWidget, &StoreDiscoverItem::clicked,
                        this, [this, productId = item.id]()
                {
                    emit navigate({Page::CATALOG_PRODUCT, productId});
                });
                ui->discoverUpcomingResultsPageLayout->addWidget(itemWidget);
            }
            ui->discoverUpcomingResultsPageLayout->addStretch();
            ui->discoverUpcomingStackedWidget->setCurrentWidget(ui->discoverUpcomingResultsPage);
        }
        else if (networkReply->error() != QNetworkReply::OperationCanceledError)
        {
            qDebug() << networkReply->error()
                     << networkReply->errorString()
                     << QString(networkReply->readAll()).toUtf8();
        }
        networkReply->deleteLater();
    });
}

void StorePage::getNews()
{
    ui->newsStackedWidget->setCurrentWidget(ui->newsLoadingPage);

    QString systemLanguage = QLocale::languageToCode(QLocale::system().language(), QLocale::ISO639Part1);
    newsReply = apiClient->getNews(0, systemLanguage, 11);
    connect(newsReply, &QNetworkReply::finished, this, [this]()
    {
        auto networkReply = newsReply;
        newsReply = nullptr;

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
}

void StorePage::getNowOnSale()
{
    ui->nowOnSaleStackedWidget->setCurrentWidget(ui->nowOnSaleLoadingPage);

    auto systemLocale = QLocale::system();
    nowOnSaleReply = apiClient->getNowOnSale(systemLocale.name(QLocale::TagSeparator::Dash),
                                             QLocale::territoryToCode(systemLocale.territory()),
                                             systemLocale.currencySymbol(QLocale::CurrencyIsoCode));
    connect(nowOnSaleReply, &QNetworkReply::finished, this, [this]()
    {
        auto networkReply = nowOnSaleReply;
        nowOnSaleReply = nullptr;

        if (networkReply->error() == QNetworkReply::NoError)
        {
            auto resultJson = QJsonDocument::fromJson(QString(networkReply->readAll()).toUtf8()).object();
            api::GetStoreNowOnSaleResponse data;
            parseGetStoreNowOnSaleResponse(resultJson, data);

            int column = 0;
            int row = 0;
            nowOnSaleSectionIds.resize(data.tabs.count());
            nowOnSaleSectionsRequested.resize(data.tabs.count());
            nowOnSaleSectionReplies.resize(data.tabs.count());

            for (const api::StoreNowOnSaleTab &dealTab : std::as_const(data.tabs))
            {
                auto dealCard = new StoreSaleCard(dealTab.bigThingy, apiClient, ui->nowOnSaleDealsScrollAreaContents);
                connect(dealCard, &StoreSaleCard::navigateToItem,
                        this, [this]()
                {
                    emit navigate({Page::ALL_GAMES, QMap<QString, QVariant>({ std::pair("discounted", true) })});
                });
                ui->nowOnSaleDealsScrollAreaContentsLayout->addWidget(dealCard, row, column, 2, 1);

                auto dealLoadingPage = new QWidget;
                dealLoadingPage->setLayout(new QVBoxLayout);
                auto dealProgressBar = new QProgressBar(dealLoadingPage);
                dealProgressBar->setMaximum(0);
                dealLoadingPage->layout()->addWidget(dealProgressBar);

                auto dealResultsPage = new QWidget;
                auto dealResultsPageLayout = new QVBoxLayout;
                dealResultsPageLayout->setContentsMargins(0, 0, 0, 0);
                dealResultsPageLayout->setSpacing(0);
                dealResultsPage->setLayout(dealResultsPageLayout);
                auto dealResultsScrollArea = new QScrollArea(dealResultsPage);
                dealResultsScrollArea->setMinimumHeight(ui->nowOnSaleDealsScrollArea->minimumHeight());
                dealResultsScrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
                dealResultsScrollArea->setWidgetResizable(true);
                auto dealResultsScrollAreaContent = new QWidget;
                auto dealResultsScrollAreaContentLayout = new QGridLayout;
                dealResultsScrollAreaContentLayout->setSpacing(24);
                dealResultsScrollAreaContent->setLayout(dealResultsScrollAreaContentLayout);
                dealResultsScrollArea->setWidget(dealResultsScrollAreaContent);
                dealResultsPageLayout->addWidget(dealResultsScrollArea);

                auto dealTabWidget = new QStackedWidget;
                dealTabWidget->addWidget(dealLoadingPage);
                dealTabWidget->addWidget(dealResultsPage);
                ui->nowOnSaleTabWidget->addTab(dealTabWidget, dealTab.title);

                nowOnSaleSectionIds[column] = dealTab.id;

                column++;
            }

            for (const api::CatalogProduct &discountedProduct : std::as_const(data.products))
            {
                if (discountedProduct.id.isNull())
                {
                    continue;
                }

                auto discountedProductItem = new SimpleProductItem(ui->nowOnSaleDealsScrollAreaContents);
                discountedProductItem->setCover(discountedProduct.coverHorizontal, apiClient);
                discountedProductItem->setTitle(discountedProduct.title);
                if (discountedProduct.price.has_value())
                {
                    const auto &price = discountedProduct.price.value();
                    auto discount = round((price.baseMoney.amount - price.finalMoney.amount) / price.baseMoney.amount * 100);
                    discountedProductItem->setPrice(price.baseMoney.amount, price.finalMoney.amount,
                                                    discount, price.baseMoney.amount == 0,
                                                    price.baseMoney.currency);
                }
                connect(this, &StorePage::ownedProductsChanged,
                        discountedProductItem, [discountedProductItem, productId = discountedProduct.id](const QSet<const QString> &ids)
                {
                    discountedProductItem->setOwned(ids.contains(productId));
                });
                discountedProductItem->setOwned(ownedProducts.contains(discountedProduct.id));
                connect(this, &StorePage::wishlistChanged,
                        discountedProductItem, [discountedProductItem, productId = discountedProduct.id](const QSet<const QString> &ids)
                {
                    discountedProductItem->setWishlisted(ids.contains(productId));
                });
                discountedProductItem->setWishlisted(wishlist.contains(discountedProduct.id));
                connect(apiClient, &api::GogApiClient::authenticated,
                        discountedProductItem, &SimpleProductItem::switchUiAuthenticatedState);
                discountedProductItem->switchUiAuthenticatedState(apiClient->isAuthenticated());
                connect(discountedProductItem, &SimpleProductItem::clicked,
                        this, [this, productId = discountedProduct.id]()
                {
                    emit navigate({Page::CATALOG_PRODUCT, productId});
                });
                ui->nowOnSaleDealsScrollAreaContentsLayout->addWidget(discountedProductItem, row, column);
                column += row;
                row = (row + 1) % 2;
            }

            if (row == 1)
            {
                column++;
                row = 0;
            }
            auto browseAllCard = new StoreSaleBrowseAllCard(ui->nowOnSaleDealsScrollAreaContents);
            connect(browseAllCard, &StoreSaleBrowseAllCard::navigateToItem,
                    this, [this]()
            {
                emit navigate({ Page::ALL_GAMES, QMap<QString, QVariant>({ std::pair("discounted", true) }) });
            });
            ui->nowOnSaleDealsScrollAreaContentsLayout->addWidget(browseAllCard, row, column, 2, 1);

            ui->nowOnSaleStackedWidget->setCurrentWidget(ui->nowOnSaleResultsPage);
        }
        else if (networkReply->error() != QNetworkReply::OperationCanceledError)
        {
            qDebug() << networkReply->error()
                     << networkReply->errorString()
                     << QString(networkReply->readAll()).toUtf8();
        }
        networkReply->deleteLater();
    });
}

void StorePage::getRecommendedDlc()
{
    ui->recommendedDlcStackedWidget->setCurrentWidget(ui->recommendedDlcLoadingPage);
    recommendedDlcReply = apiClient->getRecommendedDlcs();
    connect(recommendedDlcReply, &QNetworkReply::finished, this, [this]()
    {
        auto networkReply = recommendedDlcReply;
        recommendedDlcReply = nullptr;

        if (networkReply->error() == QNetworkReply::NoError)
        {
            auto resultJson = QJsonDocument::fromJson(QString(networkReply->readAll()).toUtf8()).object();
            api::GetStoreRecommendedDlcsResponse data;
            parseGetStoreRecommendedDlcsResponse(resultJson, data);

            for (const api::StoreProduct &item : std::as_const(data.recommendations))
            {
                auto itemWidget = new SimpleProductItem(ui->discoverUpcomingResultsPage);
                itemWidget->setCover(item.image, apiClient);
                itemWidget->setPrice(item.price.baseAmount, item.price.finalAmount, item.price.discountPercentage,
                                     item.price.free, "");
                itemWidget->setTitle(item.title);
                connect(this, &StorePage::ownedProductsChanged,
                        itemWidget, [itemWidget, productId = item.id](const QSet<const QString> &ids)
                {
                    itemWidget->setOwned(ids.contains(productId));
                });
                itemWidget->setOwned(ownedProducts.contains(item.id));
                connect(this, &StorePage::wishlistChanged,
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
}

void StorePage::initialize(const QVariant &data)
{
    ui->discoverTabWidget->setCurrentWidget(ui->discoverBestsellingTab);
    getDiscoverBestsellingGames();
    getDiscoverNewGames();
    getDiscoverUpcomingGames();
    getDealOfTheDay();
    getNowOnSale();
    getCustomSectionCDPRGames();
    getCustomSectionExclusiveGames();
    getCustomSectionGOGGames();
    getNews();
}

void StorePage::switchUiAuthenticatedState(bool authenticated)
{
    StoreBasePage::switchUiAuthenticatedState(authenticated);
    if (ownedProductsReply != nullptr)
    {
        ownedProductsReply->abort();
    }
    if (wishlistReply != nullptr)
    {
        wishlistReply->abort();
    }

    QLayoutItem *item;
    if (discoverGamesForYouReply != nullptr)
    {
        discoverGamesForYouReply->abort();
    }
    while ((item = ui->discoverGamesForYouResultsPageLayout->takeAt(0)))
    {
        auto widget = item->widget();
        delete item;
        if (widget != nullptr)
        {
            delete widget;
        }
    }
    ui->discoverTabWidget->setTabVisible(0, authenticated);

    if (recommendedDlcReply != nullptr)
    {
        recommendedDlcReply->abort();
    }
    ui->recommendedDlcLabel->setVisible(false);
    ui->recommendedDlcStackedWidget->setVisible(false);

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
        getDiscoverGamesForYou();
        getRecommendedDlc();

        ownedProductsReply = apiClient->getOwnedLicensesIds();
        connect(ownedProductsReply, &QNetworkReply::finished, this, [this]()
        {
            auto networkReply = ownedProductsReply;
            ownedProductsReply = nullptr;
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
        wishlistReply = apiClient->getWishlistIds();
        connect(wishlistReply, &QNetworkReply::finished, this, [this]()
        {
            auto networkReply = wishlistReply;
            wishlistReply = nullptr;
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
    }
    else
    {
        if (ui->discoverTabWidget->currentWidget() == ui->discoverGamesForYouTab)
        {
            ui->discoverTabWidget->setCurrentWidget(ui->discoverBestsellingTab);
        }
        ownedProducts.clear();
        emit ownedProductsChanged(ownedProducts);
        wishlist.clear();
        emit wishlistChanged(wishlist);
    }
}

void StorePage::on_showCatalogButton_clicked()
{
    if (ui->discoverTabWidget->currentWidget() == ui->discoverBestsellingTab)
    {
        emit navigate({ Page::ALL_GAMES });
    }
    else if (ui->discoverTabWidget->currentWidget() == ui->discoverNewTab)
    {
        emit navigate({ Page::ALL_GAMES, QMap<QString, QVariant>({ std::pair("releaseStatus", "new-arrival") }) });
    }
    else if (ui->discoverTabWidget->currentWidget() == ui->discoverUpcomingTab)
    {
        emit navigate({ Page::ALL_GAMES, QMap<QString, QVariant>({ std::pair("releaseStatus", "upcoming") }) });
    }
}

void StorePage::on_nowOnSaleTabWidget_currentChanged(int index)
{
    int tabIndex = index - 1;
    if (index == 0 || nowOnSaleSectionsRequested[tabIndex])
    {
        return;
    }

    nowOnSaleSectionsRequested[tabIndex] = true;
    nowOnSaleSectionReplies[tabIndex] = apiClient->getNowOnSaleSection(nowOnSaleSectionIds[tabIndex]);
    connect(nowOnSaleSectionReplies[tabIndex], &QNetworkReply::finished,
            this, [this, tabIndex, index]()
    {
        auto networkReply = nowOnSaleSectionReplies[tabIndex];
        nowOnSaleSectionReplies[tabIndex] = nullptr;

        if (networkReply->error() == QNetworkReply::NoError)
        {
            auto resultJson = QJsonDocument::fromJson(QString(networkReply->readAll()).toUtf8()).object();
            api::GetStoreNowOnSaleSectionResponse data;
            parseGetStoreNowOnSaleSectionResponse(resultJson, data);

            QStackedWidget *dealTab = static_cast<QStackedWidget *>(ui->nowOnSaleTabWidget->widget(index));
            QScrollArea *dealTabScrollArea = static_cast<QScrollArea *>(dealTab->widget(1)->layout()->itemAt(0)->widget());
            QWidget *dealTabScrollAreaContents = dealTabScrollArea->widget();
            QGridLayout *dealTabScrollAreaContentsLayout = static_cast<QGridLayout *>(dealTabScrollAreaContents->layout());
            dealTabScrollAreaContentsLayout->setAlignment(Qt::AlignLeft);

            int column = 0;
            int row = 0;
            for (std::size_t i = 0; i < 2 && data.personalizedProducts.count(); i++)
            {
                const api::StoreProduct &item = data.personalizedProducts[i];
                auto itemWidget = new SimpleProductItem(dealTabScrollAreaContents);
                itemWidget->setCover(item.image, apiClient);
                itemWidget->setTitle(item.title);
                itemWidget->setPrice(item.price.baseAmount, item.price.finalAmount,
                                     item.price.discountPercentage, item.price.free,
                                     "");
                connect(this, &StorePage::ownedProductsChanged,
                        itemWidget, [itemWidget, productId = item.id](const QSet<const QString> &ids)
                {
                    itemWidget->setOwned(ids.contains(productId));
                });
                itemWidget->setOwned(ownedProducts.contains(item.id));
                connect(this, &StorePage::wishlistChanged,
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
                dealTabScrollAreaContentsLayout->addWidget(itemWidget, row, column);
                column += row;
                row = (row + 1) % 2;
            }

            auto dealCard = new StoreSaleCard(data.bigThingy, apiClient, dealTabScrollAreaContents);
            connect(dealCard, &StoreSaleCard::navigateToItem,
                    this, [this]()
            {
                emit navigate({ Page::ALL_GAMES, QMap<QString, QVariant>({ std::pair("discounted", true) }) });
            });
            dealTabScrollAreaContentsLayout->addWidget(dealCard, 0, column, 2, 1);

            column++;
            row = 0;

            for (std::size_t i = 2; i < data.personalizedProducts.count(); i++)
            {
                const api::StoreProduct &item = data.personalizedProducts[i];
                auto itemWidget = new SimpleProductItem(dealTabScrollAreaContents);
                itemWidget->setCover(item.image, apiClient);
                itemWidget->setTitle(item.title);
                itemWidget->setPrice(item.price.baseAmount, item.price.finalAmount,
                                     item.price.discountPercentage, item.price.free,
                                     "");
                connect(this, &StorePage::ownedProductsChanged,
                        itemWidget, [itemWidget, productId = item.id](const QSet<const QString> &ids)
                {
                    itemWidget->setOwned(ids.contains(productId));
                });
                itemWidget->setOwned(ownedProducts.contains(item.id));
                connect(this, &StorePage::wishlistChanged,
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
                dealTabScrollAreaContentsLayout->addWidget(itemWidget, row, column);
                column += row;
                row = (row + 1) % 2;
            }
            dealTab->setCurrentIndex(1);
        }
        else if (networkReply->error() != QNetworkReply::OperationCanceledError)
        {
            qDebug() << networkReply->error()
                     << networkReply->errorString()
                     << QString(networkReply->readAll()).toUtf8();
        }
        networkReply->deleteLater();
    });
}


void StorePage::on_discoverTabWidget_currentChanged(int index)
{
    ui->showCatalogButton->setVisible(index > 0);
}

