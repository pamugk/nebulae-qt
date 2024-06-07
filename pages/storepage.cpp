#include "storepage.h"
#include "ui_storepage.h"

#include <QJsonDocument>
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
    BasePage(parent),
    apiClient(nullptr),
    customSectionCDPRReply(nullptr),
    customSectionExclusivesReply(nullptr),
    customSectionGOGReply(nullptr),
    discoverBestsellingReply(nullptr),
    discoverNewReply(nullptr),
    discoverUpcomingReply(nullptr),
    newsReply(nullptr),
    nowOnSaleReply(nullptr),
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
    if (discoverBestsellingReply != nullptr)
    {
        discoverBestsellingReply->abort();
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
            nowOnSaleReply->abort();
        }
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
                auto itemWidget = new SimpleProductItem(item.product.id, ui->customSectionCDPRScrollAreaContents);
                itemWidget->setCover(item.product.image, apiClient);
                itemWidget->setTitle(item.product.title);
                itemWidget->setPrice(item.product.price.baseAmount, item.product.price.finalAmount,
                                     item.product.price.discountPercentage, item.product.price.free, "");
                connect(apiClient, &api::GogApiClient::authenticated,
                        itemWidget, &SimpleProductItem::switchUiAuthenticatedState);
                itemWidget->switchUiAuthenticatedState(apiClient->isAuthenticated());
                connect(itemWidget, &SimpleProductItem::navigateToProduct,
                        this, [this](unsigned long long productId)
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
                auto itemWidget = new SimpleProductItem(item.product.id, ui->customSectionExclusivesResultsScrollAreaContents);
                itemWidget->setCover(item.product.image, apiClient);
                itemWidget->setTitle(item.product.title);
                itemWidget->setPrice(item.product.price.baseAmount, item.product.price.finalAmount,
                                     item.product.price.discountPercentage, item.product.price.free, "");
                connect(apiClient, &api::GogApiClient::authenticated,
                        itemWidget, &SimpleProductItem::switchUiAuthenticatedState);
                itemWidget->switchUiAuthenticatedState(apiClient->isAuthenticated());
                connect(itemWidget, &SimpleProductItem::navigateToProduct,
                        this, [this](unsigned long long productId)
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
                auto itemWidget = new SimpleProductItem(item.product.id, ui->customSectionCDPRScrollAreaContents);
                itemWidget->setCover(item.product.image, apiClient);
                itemWidget->setTitle(item.product.title);
                itemWidget->setPrice(item.product.price.baseAmount, item.product.price.finalAmount,
                                     item.product.price.discountPercentage, item.product.price.free, "");
                connect(apiClient, &api::GogApiClient::authenticated,
                        itemWidget, &SimpleProductItem::switchUiAuthenticatedState);
                itemWidget->switchUiAuthenticatedState(apiClient->isAuthenticated());
                connect(itemWidget, &SimpleProductItem::navigateToProduct,
                        this, [this](unsigned long long productId)
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
                auto itemWidget = new StoreDiscoverItem(item.id.toULongLong(), ui->discoverBestsellingResultsPage);
                itemWidget->setCover(item.coverHorizontal, apiClient);
                itemWidget->setTitle(item.title);
                itemWidget->setPrice(item.price.baseMoney.amount, item.price.finalMoney.amount,
                                     item.price.discount.isNull()
                                        ? 0
                                        : round(100. * (item.price.baseMoney.amount - item.price.finalMoney.amount) / item.price.baseMoney.amount),
                                     item.price.finalMoney.amount == 0, item.price.finalMoney.currency);
                connect(apiClient, &api::GogApiClient::authenticated,
                        itemWidget, &StoreDiscoverItem::switchUiAuthenticatedState);
                itemWidget->switchUiAuthenticatedState(apiClient->isAuthenticated());
                connect(itemWidget, &StoreDiscoverItem::navigateToProduct,
                        this, [this](unsigned long long productId)
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
                auto itemWidget = new StoreDiscoverItem(item.id, ui->discoverNewResultsPage);
                itemWidget->setCover(item.image, apiClient);
                itemWidget->setTitle(item.title);
                itemWidget->setPrice(item.price.baseAmount, item.price.finalAmount, item.price.discountPercentage,
                                     item.price.free, "");
                connect(apiClient, &api::GogApiClient::authenticated,
                        itemWidget, &StoreDiscoverItem::switchUiAuthenticatedState);
                itemWidget->switchUiAuthenticatedState(apiClient->isAuthenticated());
                connect(itemWidget, &StoreDiscoverItem::navigateToProduct,
                        this, [this](unsigned long long productId)
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
                auto itemWidget = new StoreDiscoverItem(item.id, ui->discoverUpcomingResultsPage);
                itemWidget->setCover(item.image, apiClient);
                itemWidget->setPreorder(item.preorder);
                itemWidget->setPrice(item.price.baseAmount, item.price.finalAmount, item.price.discountPercentage,
                                     item.price.free, "");
                itemWidget->setTitle(item.title);
                connect(apiClient, &api::GogApiClient::authenticated,
                        itemWidget, &StoreDiscoverItem::switchUiAuthenticatedState);
                itemWidget->switchUiAuthenticatedState(apiClient->isAuthenticated());
                connect(itemWidget, &StoreDiscoverItem::navigateToProduct,
                        this, [this](unsigned long long productId)
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
                    emit navigate({Page::ALL_GAMES});
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

                auto discountedProductItem = new SimpleProductItem(discountedProduct.id.toULongLong(), ui->nowOnSaleDealsScrollAreaContents);
                discountedProductItem->setCover(discountedProduct.coverHorizontal, apiClient);
                discountedProductItem->setTitle(discountedProduct.title);
                auto discount = round((discountedProduct.price.baseMoney.amount - discountedProduct.price.finalMoney.amount) / discountedProduct.price.baseMoney.amount * 100);
                discountedProductItem->setPrice(discountedProduct.price.baseMoney.amount, discountedProduct.price.finalMoney.amount,
                                                discount, discountedProduct.price.baseMoney.amount == 0,
                                                discountedProduct.price.baseMoney.currency);
                connect(apiClient, &api::GogApiClient::authenticated,
                        discountedProductItem, &SimpleProductItem::switchUiAuthenticatedState);
                discountedProductItem->switchUiAuthenticatedState(apiClient->isAuthenticated());
                connect(discountedProductItem, &SimpleProductItem::navigateToProduct,
                        this, [this](unsigned long long productId)
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
                emit navigate({Page::ALL_GAMES});
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

void StorePage::initialize(const QVariant &data)
{
    getCustomSectionCDPRGames();
    getCustomSectionExclusiveGames();
    getCustomSectionGOGGames();
    ui->discoverTabWidget->setCurrentWidget(ui->discoverBestsellingTab);
    getDiscoverBestsellingGames();
    getDiscoverNewGames();
    getDiscoverUpcomingGames();
    getNews();
    getNowOnSale();
}

void StorePage::switchUiAuthenticatedState(bool authenticated)
{

}

void StorePage::on_showCatalogButton_clicked()
{
    emit navigate({Page::ALL_GAMES});
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
                auto itemWidget = new SimpleProductItem(item.id, dealTabScrollAreaContents);
                itemWidget->setCover(item.image, apiClient);
                itemWidget->setTitle(item.title);
                itemWidget->setPrice(item.price.baseAmount, item.price.finalAmount,
                                     item.price.discountPercentage, item.price.free,
                                     "");
                connect(apiClient, &api::GogApiClient::authenticated,
                        itemWidget, &SimpleProductItem::switchUiAuthenticatedState);
                itemWidget->switchUiAuthenticatedState(apiClient->isAuthenticated());
                connect(itemWidget, &SimpleProductItem::navigateToProduct,
                        this, [this](unsigned long long productId)
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
                emit navigate({Page::ALL_GAMES});
            });
            dealTabScrollAreaContentsLayout->addWidget(dealCard, 0, column, 2, 1);

            column++;
            row = 0;

            for (std::size_t i = 2; i < data.personalizedProducts.count(); i++)
            {
                const api::StoreProduct &item = data.personalizedProducts[i];
                auto itemWidget = new SimpleProductItem(item.id, dealTabScrollAreaContents);
                itemWidget->setCover(item.image, apiClient);
                itemWidget->setTitle(item.title);
                itemWidget->setPrice(item.price.baseAmount, item.price.finalAmount,
                                     item.price.discountPercentage, item.price.free,
                                     "");
                connect(apiClient, &api::GogApiClient::authenticated,
                        itemWidget, &SimpleProductItem::switchUiAuthenticatedState);
                itemWidget->switchUiAuthenticatedState(apiClient->isAuthenticated());
                connect(itemWidget, &SimpleProductItem::navigateToProduct,
                        this, [this](unsigned long long productId)
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

