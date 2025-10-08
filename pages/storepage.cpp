#include "storepage.h"
#include "ui_storepage.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QNetworkReply>

#include "../api/utils/catalogserialization.h"
#include "../api/utils/newsserialization.h"
#include "../api/utils/storeserialization.h"
#include "../widgets/simpleproductitem.h"
#include "../widgets/storediscoveritem.h"
#include "../widgets/storesalebrowseallcard.h"
#include "../widgets/newsitemtile.h"

StorePage::StorePage(QWidget *parent) :
    StoreBasePage(Page::STORE, parent),
    apiClient(nullptr),
    ui(new Ui::StorePage)
{
    ui->setupUi(this);
}

StorePage::~StorePage()
{
    delete ui;
}

void StorePage::setApiClient(api::GogApiClient *apiClient)
{
    this->apiClient = apiClient;
}

/*void StorePage::getCustomSectionCDPRGames()
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
    /*customSectionExclusivesReply = apiClient->getStoreCustomSection("eea11712-458e-11ee-9787-fa163eebc216");
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
    /*discoverGamesForYouReply = apiClient->getStoreDiscoverGamesForYou();
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
    /*discoverNewReply = apiClient->getStoreDiscoverNewGames();
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
    /*discoverUpcomingReply = apiClient->getStoreDiscoverUpcomingGames();
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
    /*recommendedDlcReply = apiClient->getRecommendedDlcs();
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
}*/

void StorePage::getSection(const QString &id, const QString &type)
{
    QWidget *sectionWidget = new QWidget(ui->landingScrollAreaContents);
    ui->landingScrollAreaContentsLayout->addWidget(sectionWidget);

    const auto systemLocale = QLocale::system();
    const auto sectionReply = apiClient->getStoreSection(id, systemLocale.name(QLocale::TagSeparator::Dash),
                                                        QLocale::territoryToCode(systemLocale.territory()),
                                                        systemLocale.currencySymbol(QLocale::CurrencyIsoCode));
    connect(this, &QObject::destroyed, sectionReply, &QNetworkReply::abort);
    connect(sectionReply, &QNetworkReply::finished, this, [this, sectionReply, sectionWidget]()
    {
        if (sectionReply->error() == QNetworkReply::NoError)
        {
            auto resultJson = QJsonDocument::fromJson(QString(sectionReply->readAll()).toUtf8()).object();
            api::GetStoreProductsSectionResponse data;
            parseGetStoreProductsSectionResponse(resultJson, data);
            if (data.items.isEmpty())
            {
                ui->landingScrollAreaContentsLayout->removeWidget(sectionWidget);
                sectionWidget->deleteLater();
            }
            else
            {
                sectionWidget->setLayout(new QVBoxLayout());
                auto sectionScrollArea = new QScrollArea(sectionWidget);
                sectionScrollArea->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
                sectionScrollArea->setMinimumHeight(273);
                auto sectionScrollAreaContents = new QWidget(sectionScrollArea);
                auto sectionScrollAreaContentsLayout = new QHBoxLayout(sectionScrollAreaContents);
                sectionScrollAreaContentsLayout->setContentsMargins(6, 0, 6, 6);
                sectionScrollAreaContentsLayout->setSpacing(24);
                for (const api::CatalogProduct &item : std::as_const(data.items))
                {
                    auto itemWidget = new SimpleProductItem(sectionScrollArea);
                    itemWidget->setCover(item.coverHorizontal, apiClient);
                    itemWidget->setTitle(item.title);
                    if (item.price.has_value())
                    {
                        itemWidget->setPrice(item.price->baseMoney.amount, item.price->finalMoney.amount,
                                             std::round((item.price->finalMoney.amount / item.price->baseMoney.amount) * 100),
                                             item.price->finalMoney.amount == 0, "");
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
                            itemWidget, &SimpleProductItem::switchUiAuthenticatedState);
                    itemWidget->switchUiAuthenticatedState(apiClient->isAuthenticated());
                    connect(itemWidget, &SimpleProductItem::clicked,
                            this, [this, productId = item.id]()
                    {
                        emit navigate({Page::CATALOG_PRODUCT, productId});
                    });
                    sectionScrollAreaContentsLayout->addWidget(itemWidget);
                }
                sectionScrollAreaContentsLayout->addStretch();
                sectionScrollArea->setWidget(sectionScrollAreaContents);
                sectionWidget->layout()->addWidget(sectionScrollArea);

                if (!data.title.isEmpty())
                {
                    auto titleLabel = new QLabel(data.title, ui->landingScrollAreaContents);
                    titleLabel->setStyleSheet(QStringLiteral("font: 700 12pt; padding: 16px 0; border-bottom: 1px solid #bfbfbf;"));
                    ui->landingScrollAreaContentsLayout->insertWidget(ui->landingScrollAreaContentsLayout->indexOf(sectionWidget), titleLabel);
                }
            }
        }
        else if (sectionReply->error() != QNetworkReply::OperationCanceledError)
        {
            qDebug() << sectionReply->error()
                     << sectionReply->errorString()
                     << QString(sectionReply->readAll()).toUtf8();
            ui->landingScrollAreaContentsLayout->removeWidget(sectionWidget);
            sectionWidget->deleteLater();
        }

        sectionReply->deleteLater();
    });
}

void StorePage::getSections()
{
    ui->landingStackedWidget->setCurrentWidget(ui->landingLoadingPage);
    QLayoutItem *sectionItem;
    while ((sectionItem = ui->landingScrollAreaContentsLayout->takeAt(0)))
    {
        auto widget = sectionItem->widget();
        delete sectionItem;
        if (widget != nullptr)
        {
            delete widget;
        }
    }

    const auto systemLocale = QLocale::system();
    const auto sectionsReply = apiClient->getStoreSections(systemLocale.name(QLocale::TagSeparator::Dash),
                                                           QLocale::territoryToCode(systemLocale.territory()),
                                                           systemLocale.currencySymbol(QLocale::CurrencyIsoCode));
    connect(this, &QObject::destroyed, sectionsReply, &QNetworkReply::abort);
    connect(sectionsReply, &QNetworkReply::finished, this, [this, sectionsReply]()
    {
        if (sectionsReply->error() == QNetworkReply::NoError)
        {
            auto resultJson = QJsonDocument::fromJson(QString(sectionsReply->readAll()).toUtf8()).object();
            api::GetStoreSectionsResponse data;
            parseGetStoreSectionsResponse(resultJson, data);
            for (const auto &section: std::as_const(data.sections))
            {
                if (section.sectionType == QLatin1StringView("PRODUCTS_SECTION"))
                {
                    getSection(section.id, section.sectionType);
                }
            }

            ui->landingStackedWidget->setCurrentWidget(ui->landingResultPage);
        }
        else if (sectionsReply->error() != QNetworkReply::OperationCanceledError)
        {
            qDebug() << sectionsReply->error()
                     << sectionsReply->errorString()
                     << QString(sectionsReply->readAll()).toUtf8();
            ui->landingStackedWidget->setCurrentWidget(ui->landingErrorPage);
        }

        sectionsReply->deleteLater();
    });
}

void StorePage::initialize(const QVariant &data)
{
    getSections();
}

void StorePage::switchUiAuthenticatedState(bool authenticated)
{
    emit authenticationStateChanged();
    StoreBasePage::switchUiAuthenticatedState(authenticated);

    if (authenticated)
    {
        const auto ownedProductsReply = apiClient->getOwnedLicensesIds();
        connect(this, &QObject::destroyed, ownedProductsReply, &QNetworkReply::abort);
        connect(this, &StorePage::authenticationStateChanged, ownedProductsReply, &QNetworkReply::abort);
        connect(ownedProductsReply, &QNetworkReply::finished, this, [this, ownedProductsReply]()
        {
            if (ownedProductsReply->error() == QNetworkReply::NoError)
            {
                auto resultJson = QJsonDocument::fromJson(QString(ownedProductsReply->readAll()).toUtf8());
                auto ownedProducts = resultJson.toVariant().toList();
                for (const QVariant &id : std::as_const(ownedProducts))
                {
                    this->ownedProducts.insert(id.toString());
                }
                emit ownedProductsChanged(this->ownedProducts);
            }
            else if (ownedProductsReply->error() != QNetworkReply::OperationCanceledError)
            {
                qDebug() << ownedProductsReply->error()
                         << ownedProductsReply->errorString()
                         << QString(ownedProductsReply->readAll()).toUtf8();
            }

            ownedProductsReply->deleteLater();
        });

        const auto wishlistReply = apiClient->getWishlistIds();
        connect(this, &QObject::destroyed, wishlistReply, &QNetworkReply::abort);
        connect(this, &StorePage::authenticationStateChanged, wishlistReply, &QNetworkReply::abort);
        connect(wishlistReply, &QNetworkReply::finished, this, [this, wishlistReply]()
        {
            if (wishlistReply->error() == QNetworkReply::NoError)
            {
                auto resultJson = QJsonDocument::fromJson(QString(wishlistReply->readAll()).toUtf8());
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
            else if (wishlistReply->error() != QNetworkReply::OperationCanceledError)
            {
                qDebug() << wishlistReply->error()
                         << wishlistReply->errorString()
                         << QString(wishlistReply->readAll()).toUtf8();
            }

            wishlistReply->deleteLater();
        });
    }
    else
    {
        ownedProducts.clear();
        emit ownedProductsChanged(ownedProducts);
        wishlist.clear();
        emit wishlistChanged(wishlist);
    }
}

/*void StorePage::on_showCatalogButton_clicked()
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
}*/

