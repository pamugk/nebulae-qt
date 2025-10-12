#include "storepage.h"
#include "ui_storepage.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QNetworkReply>
#include <QTabWidget>

#include "../api/utils/catalogserialization.h"
#include "../api/utils/storeserialization.h"
#include "../widgets/simpleproductitem.h"
#include "../widgets/storediscoveritem.h"
#include "../widgets/storesalebrowseallcard.h"
#include "../widgets/storesalecard.h"
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

void StorePage::getNowOnSale()
{
    QTabWidget *nowOnSaleTabWidget = new QTabWidget(ui->landingScrollAreaContents);
    connect(nowOnSaleTabWidget, &QTabWidget::currentChanged, this, [this, nowOnSaleTabWidget](int index)
    {
        int tabIndex = index - 1;
        if (index == 0 || nowOnSaleSectionsRequested[tabIndex])
        {
            return;
        }

        nowOnSaleSectionsRequested[tabIndex] = true;
        auto nowOnSaleSectionReply = apiClient->getNowOnSaleSection(nowOnSaleSectionsIds[tabIndex]);
        connect(this, &QObject::destroyed, nowOnSaleSectionReply, &QNetworkReply::abort);
        connect(nowOnSaleSectionReply, &QNetworkReply::finished,
                this, [this, tabIndex, index, nowOnSaleSectionReply, nowOnSaleTabWidget]()
        {
            if (nowOnSaleSectionReply->error() == QNetworkReply::NoError)
            {
                auto resultJson = QJsonDocument::fromJson(QString(nowOnSaleSectionReply->readAll()).toUtf8()).object();
                api::GetStoreNowOnSaleSectionResponse data;
                parseGetStoreNowOnSaleSectionResponse(resultJson, data);

                QStackedWidget *dealTab = static_cast<QStackedWidget *>(nowOnSaleTabWidget->widget(index));
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
            else if (nowOnSaleSectionReply->error() != QNetworkReply::OperationCanceledError)
            {
                qDebug() << nowOnSaleSectionReply->error()
                         << nowOnSaleSectionReply->errorString()
                         << QString(nowOnSaleSectionReply->readAll()).toUtf8();
            }

            nowOnSaleSectionReply->deleteLater();
        });
    });
    nowOnSaleTabWidget->setVisible(false);
    ui->landingScrollAreaContentsLayout->addWidget(nowOnSaleTabWidget);

    auto systemLocale = QLocale::system();
    const auto nowOnSaleReply = apiClient->getNowOnSale(systemLocale.name(QLocale::TagSeparator::Dash),
                                                        QLocale::territoryToCode(systemLocale.territory()),
                                                        systemLocale.currencySymbol(QLocale::CurrencyIsoCode));
    connect(this, &QObject::destroyed, nowOnSaleReply, &QNetworkReply::abort);
    connect(nowOnSaleReply, &QNetworkReply::finished, this, [this, nowOnSaleReply, nowOnSaleTabWidget]()
    {
        if (nowOnSaleReply->error() == QNetworkReply::NoError)
        {
            auto resultJson = QJsonDocument::fromJson(QString(nowOnSaleReply->readAll()).toUtf8()).object();
            api::GetStoreNowOnSaleResponse data;
            parseGetStoreNowOnSaleResponse(resultJson, data);

            QWidget *nowOnSaleDealsTab = new QWidget();
            nowOnSaleDealsTab->setLayout(new QVBoxLayout());
            QScrollArea *nowOnSaleDealsScrollArea = new QScrollArea(nowOnSaleDealsTab);
            nowOnSaleDealsScrollArea->setMinimumSize(548, 474);
            nowOnSaleDealsScrollArea->setWidgetResizable(true);
            QWidget *nowOnSaleDealsScrollAreaContents = new QWidget(nowOnSaleDealsScrollArea);
            QGridLayout *nowOnSaleDealsScrollAreaContentsLayout = new QGridLayout(nowOnSaleDealsScrollAreaContents);
            nowOnSaleDealsScrollAreaContentsLayout->setHorizontalSpacing(24);
            nowOnSaleDealsScrollAreaContentsLayout->setContentsMargins(6, 6, 6, 6);
            nowOnSaleDealsScrollAreaContentsLayout->setVerticalSpacing(24);
            nowOnSaleDealsScrollAreaContents->setLayout(nowOnSaleDealsScrollAreaContentsLayout);
            nowOnSaleDealsScrollArea->setWidget(nowOnSaleDealsScrollAreaContents);
            nowOnSaleDealsTab->layout()->addWidget(nowOnSaleDealsScrollArea);
            nowOnSaleTabWidget->addTab(nowOnSaleDealsTab, tr("Featured deals"));

            int column = 0;
            int row = 0;
            nowOnSaleSectionsIds.resize(data.tabs.count());
            nowOnSaleSectionsRequested.resize(data.tabs.count());

            for (const api::StoreNowOnSaleTab &dealTab : std::as_const(data.tabs))
            {
                auto dealCard = new StoreSaleCard(dealTab.bigThingy, apiClient, nowOnSaleDealsScrollAreaContents);
                connect(dealCard, &StoreSaleCard::navigateToItem,
                        this, [this]()
                {
                    emit navigate({Page::ALL_GAMES, QMap<QString, QVariant>({ std::pair("discounted", true) })});
                });
                nowOnSaleDealsScrollAreaContentsLayout->addWidget(dealCard, row, column, 2, 1);

                auto dealLoadingPage = new QWidget();
                dealLoadingPage->setLayout(new QVBoxLayout());
                auto dealProgressBar = new QProgressBar(dealLoadingPage);
                dealProgressBar->setMaximum(0);
                dealLoadingPage->layout()->addWidget(dealProgressBar);

                auto dealResultsPage = new QWidget();
                auto dealResultsPageLayout = new QVBoxLayout();
                dealResultsPageLayout->setContentsMargins(0, 0, 0, 0);
                dealResultsPageLayout->setSpacing(0);
                dealResultsPage->setLayout(dealResultsPageLayout);
                auto dealResultsScrollArea = new QScrollArea(dealResultsPage);
                dealResultsScrollArea->setMinimumHeight(nowOnSaleDealsScrollArea->minimumHeight());
                dealResultsScrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
                dealResultsScrollArea->setWidgetResizable(true);
                auto dealResultsScrollAreaContent = new QWidget();
                auto dealResultsScrollAreaContentLayout = new QGridLayout();
                dealResultsScrollAreaContentLayout->setSpacing(24);
                dealResultsScrollAreaContent->setLayout(dealResultsScrollAreaContentLayout);
                dealResultsScrollArea->setWidget(dealResultsScrollAreaContent);
                dealResultsPageLayout->addWidget(dealResultsScrollArea);

                auto dealTabWidget = new QStackedWidget();
                dealTabWidget->addWidget(dealLoadingPage);
                dealTabWidget->addWidget(dealResultsPage);
                nowOnSaleTabWidget->addTab(dealTabWidget, dealTab.title);

                nowOnSaleSectionsIds[column] = dealTab.id;

                column++;
            }

            for (const api::CatalogProduct &discountedProduct : std::as_const(data.products))
            {
                if (discountedProduct.id.isNull())
                {
                    continue;
                }

                auto discountedProductItem = new SimpleProductItem(nowOnSaleDealsScrollAreaContents);
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
                nowOnSaleDealsScrollAreaContentsLayout->addWidget(discountedProductItem, row, column);
                column += row;
                row = (row + 1) % 2;
            }

            if (row == 1)
            {
                column++;
                row = 0;
            }
            auto browseAllCard = new StoreSaleBrowseAllCard(nowOnSaleDealsScrollAreaContents);
            connect(browseAllCard, &StoreSaleBrowseAllCard::navigateToItem,
                    this, [this]()
            {
                emit navigate({ Page::ALL_GAMES, QMap<QString, QVariant>({ std::pair("discounted", true) }) });
            });
            nowOnSaleDealsScrollAreaContentsLayout->addWidget(browseAllCard, row, column, 2, 1);

            nowOnSaleTabWidget->setVisible(true);
            auto titleLabel = new QLabel(tr("Now on sale"), ui->landingScrollAreaContents);
            titleLabel->setStyleSheet(QStringLiteral("font: 700 12pt; padding: 16px 0; border-bottom: 1px solid #bfbfbf;"));
            ui->landingScrollAreaContentsLayout->insertWidget(ui->landingScrollAreaContentsLayout->indexOf(nowOnSaleTabWidget), titleLabel);
        }
        else if (nowOnSaleReply->error() != QNetworkReply::OperationCanceledError)
        {
            qDebug() << nowOnSaleReply->error()
                     << nowOnSaleReply->errorString()
                     << QString(nowOnSaleReply->readAll()).toUtf8();
            ui->landingScrollAreaContentsLayout->removeWidget(nowOnSaleTabWidget);
            nowOnSaleTabWidget->deleteLater();
        }
        nowOnSaleReply->deleteLater();
    });
}

/*
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
    connect(sectionReply, &QNetworkReply::finished, this, [this, sectionReply, sectionWidget, type]()
    {
        if (sectionReply->error() == QNetworkReply::NoError)
        {
            if (type == QLatin1StringView("PRODUCTS_SECTION"))
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
                                                 100 - std::round((item.price->finalMoney.amount / item.price->baseMoney.amount) * 100),
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
            else if (type == QLatin1StringView("NEWS_SECTION"))
            {
                auto resultJson = QJsonDocument::fromJson(QString(sectionReply->readAll()).toUtf8()).object();
                api::GetStoreNewsSectionResponse data;
                parseGetStoreNewsSectionResponse(resultJson, data);

                sectionWidget->setLayout(new QVBoxLayout());
                auto sectionScrollArea = new QScrollArea(sectionWidget);
                sectionScrollArea->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
                sectionScrollArea->setMinimumHeight(296);
                auto sectionScrollAreaContents = new QWidget(sectionScrollArea);
                auto sectionScrollAreaContentsLayout = new QHBoxLayout(sectionScrollAreaContents);
                sectionScrollAreaContentsLayout->setContentsMargins(6, 0, 6, 6);

                bool primary = true;
                for (const api::NewsItem &item : std::as_const(data.items))
                {
                    auto itemTile = new NewsItemTile(item, primary, apiClient, sectionScrollAreaContents);
                    primary = false;
                    connect(itemTile, &NewsItemTile::navigateToNewsItem, this,
                            [this](unsigned long long newsId)
                    {
                        emit navigate({ Page::NEWS, newsId });
                    });
                    sectionScrollAreaContentsLayout->addWidget(itemTile);
                }
                sectionScrollAreaContentsLayout->setSpacing(24);
                sectionScrollAreaContentsLayout->addStretch();

                sectionScrollArea->setWidget(sectionScrollAreaContents);
                sectionWidget->layout()->addWidget(sectionScrollArea);

                auto titleLabel = new QLabel(tr("News"), ui->landingScrollAreaContents);
                titleLabel->setStyleSheet(QStringLiteral("font: 700 12pt; padding: 16px 0; border-bottom: 1px solid #bfbfbf;"));
                ui->landingScrollAreaContentsLayout->insertWidget(ui->landingScrollAreaContentsLayout->indexOf(sectionWidget), titleLabel);
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
                if (section.sectionType == QLatin1StringView("PRODUCTS_SECTION")
                    || section.sectionType == QLatin1StringView("NEWS_SECTION"))
                {
                    getSection(section.id, section.sectionType);
                }
                else if (section.sectionType == QLatin1StringView("TABBED_SECTION"))
                {
                    getNowOnSale();
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
