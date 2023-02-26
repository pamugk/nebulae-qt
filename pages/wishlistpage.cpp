#include "wishlistpage.h"
#include "ui_wishlistpage.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QMenu>
#include <QMenuBar>
#include <QNetworkReply>

#include "../api/utils/serialization.h"
#include "../widgets/wishlistitem.h"

WishlistPage::WishlistPage(QWidget *parent) :
    BasePage(parent),
    ui(new Ui::WishlistPage)
{
    ui->setupUi(this);

    auto menuBar = new QMenuBar(this);
    auto genresMenu = menuBar->addMenu("GENRES");
    genresMenu->addAction("SIMULATION");
    genresMenu->addAction("INDIE");
    genresMenu->addAction("ACTION");
    genresMenu->addAction("ADVENTURE");
    genresMenu->addAction("ROLE-PLAYING");
    genresMenu->addAction("RACING");
    genresMenu->addAction("STRATEGY");
    genresMenu->addAction("SHOOTER");
    auto systemMenu = menuBar->addMenu("SYSTEM");
    systemMenu->addAction("Windows");
    systemMenu->addAction("Mac OS");
    systemMenu->addAction("Linux");
    auto languageMenu = menuBar->addMenu("LANGUAGE");
    languageMenu->addAction("ENGLISH");
    auto featureMenu = menuBar->addMenu("FEATURES");
    featureMenu->addAction("SINGLE-PLAYER");
    featureMenu->addAction("ACHIEVEMENTS");
    featureMenu->addAction("CONTROLLER SUPPORT");
    featureMenu->addAction("CLOUD SAVES");
    featureMenu->addAction("OVERLAY");
    featureMenu->addAction("SINGLE-PLAYER");
    featureMenu->addAction("SINGLE-PLAYER");
    menuBar->addMenu("PRICE");

    ui->filtersLayout->addWidget(menuBar);
}

WishlistPage::~WishlistPage()
{
    delete ui;
}

void WishlistPage::setApiClient(GogApiClient *apiClient)
{
    this->apiClient = apiClient;
}

void WishlistPage::clear()
{
    ui->titleLabel->setText("WISHLISTED TITLES");
    ui->contentsStack->setCurrentWidget(ui->loaderPage);
    while (!ui->resultsScrollAreaContentsLayout->isEmpty())
    {
        ui->resultsScrollAreaContentsLayout->removeItem(ui->resultsScrollAreaContentsLayout->itemAt(0));
    }
}

void WishlistPage::initialize()
{
    ui->contentsStack->setCurrentWidget(ui->loaderPage);
    auto networkReply = apiClient->getWishlist();
    connect(networkReply, &QNetworkReply::finished, this, [=](){
        if (networkReply->error() == QNetworkReply::NoError)
        {
            auto resultJson = QJsonDocument::fromJson(QString(networkReply->readAll()).toUtf8()).object();
            GetWishlistResponse data;
            parseSearchWishlistResponse(resultJson, data);
            Product product;
            foreach (product, data.products)
            {
                ui->resultsScrollAreaContentsLayout->addWidget(new WishlistItem(product, apiClient, ui->resultsScrollAreaContents));
            }
            ui->titleLabel->setText(QString("WISHLISTED TITLES (%1)").arg(data.totalProducts));
            ui->contentsStack->setCurrentWidget(ui->resultsPage);
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
