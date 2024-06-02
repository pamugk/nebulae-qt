#include "storepage.h"
#include "ui_storepage.h"

#include <cmath>
#include <QJsonDocument>
#include <QNetworkReply>

#include "../api/utils/catalogserialization.h"
#include "../api/utils/newsserialization.h"
#include "../api/utils/storeserialization.h"
#include "../widgets/storediscoveritem.h"
#include "../widgets/newsitemtile.h"

StorePage::StorePage(QWidget *parent) :
    BasePage(parent),
    apiClient(nullptr),
    discoverBestsellingReply(nullptr),
    discoverNewReply(nullptr),
    discoverUpcomingReply(nullptr),
    newsReply(nullptr),
    ui(new Ui::StorePage)
{
    ui->setupUi(this);
}

StorePage::~StorePage()
{
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
    delete ui;
}

void StorePage::setApiClient(api::GogApiClient *apiClient)
{
    this->apiClient = apiClient;
}

void StorePage::getDiscoverBestsellingGames()
{
    auto systemLocale = QLocale::system();
    ui->discoverBestsellingStackedWidget->setCurrentWidget(ui->discoverBestsellingLoadingPage);
    discoverBestsellingReply = apiClient->searchCatalog({ "trending", false }, {},
                                                        QLocale::territoryToCode(systemLocale.territory()),
                                                        QLocale::languageToCode(systemLocale.language(), QLocale::ISO639Part1),
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
                connect(itemWidget, &StoreDiscoverItem::navigateToProduct,
                        this, [this](unsigned long long productId)
                {
                    emit navigate({Page::CATALOG_PRODUCT_PAGE, productId});
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

            for (const api::StoreDiscoveredProduct &item : std::as_const(data.personalizedProducts))
            {
                auto itemWidget = new StoreDiscoverItem(item.id, ui->discoverNewResultsPage);
                itemWidget->setCover(item.image, apiClient);
                itemWidget->setTitle(item.title);
                itemWidget->setPrice(item.price.baseAmount, item.price.finalAmount, item.price.discountPercentage,
                                     item.price.free, "");
                connect(itemWidget, &StoreDiscoverItem::navigateToProduct,
                        this, [this](unsigned long long productId)
                {
                    emit navigate({Page::CATALOG_PRODUCT_PAGE, productId});
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

            for (const api::StoreDiscoveredProduct &item : std::as_const(data.personalizedProducts))
            {
                auto itemWidget = new StoreDiscoverItem(item.id, ui->discoverUpcomingResultsPage);
                itemWidget->setCover(item.image, apiClient);
                itemWidget->setPreorder(item.preorder);
                itemWidget->setPrice(item.price.baseAmount, item.price.finalAmount, item.price.discountPercentage,
                                     item.price.free, "");
                itemWidget->setTitle(item.title);
                connect(itemWidget, &StoreDiscoverItem::navigateToProduct,
                        this, [this](unsigned long long productId)
                {
                    emit navigate({Page::CATALOG_PRODUCT_PAGE, productId});
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

void StorePage::initialize(const QVariant &data)
{
    ui->discoverTabWidget->setCurrentWidget(ui->discoverBestsellingTab);
    getDiscoverBestsellingGames();
    getDiscoverNewGames();
    getDiscoverUpcomingGames();
    getNews();
}

void StorePage::switchUiAuthenticatedState(bool authenticated)
{

}


void StorePage::on_showCatalogButton_clicked()
{
    emit navigate({Page::ALL_GAMES});
}

