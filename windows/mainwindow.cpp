#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include "../pages/allgamespage.h"
#include "../pages/catalogproductpage.h"
#include "../pages/orderspage.h"
#include "../pages/ownedgamepage.h"
#include "../pages/ownedgamespage.h"
#include "../pages/storepage.h"
#include "../pages/wishlistpage.h"

#include "authdialog.h"
#include "searchdialog.h"
#include "settingsdialog.h"

MainWindow::MainWindow(api::GogApiClient * apiClient,
                       SettingsManager *settingsManager,
                       QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow),
      apiClient(apiClient),
      settingsManager(settingsManager)
{
    ui->setupUi(this);

    ui->loginButton->setVisible(!apiClient->isAuthenticated());
    connect(apiClient, &api::GogApiClient::authenticated, this, [this]{
        ui->loginButton->setVisible(false);
    });
    connect(ui->loginButton, &QPushButton::clicked, apiClient, &api::GogApiClient::grant);
    connect(apiClient, &api::GogApiClient::authorize, this, [this](const QUrl &authUrl){
        AuthDialog dialog(this);
        dialog.setUrl(authUrl);
        dialog.exec();
    });

    navigationHistory.push(NavigationDestination { Page::STORE });
    BasePage *startPage = new StorePage(ui->scaffold);
    startPage->setApiClient(apiClient);
    ui->scaffoldLayout->addWidget(startPage, 1, 1);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::navigate(Page newPage, const QVariant &data)
{
    NavigationDestination currentDestination = navigationHistory.top();
    if (currentDestination.page != newPage) {
        BasePage *nextPage = nullptr;
        switch (newPage) {
        case DISCOVER:
        case RECENT:
        case STORE:
            nextPage = new StorePage(ui->scaffold);
            break;
        case ALL_GAMES:
            nextPage = new AllGamesPage(ui->scaffold);
            break;
        case WISHLIST:
            nextPage = new WishlistPage(ui->scaffold);
            break;
        case ORDER_HISTORY:
            nextPage = new OrdersPage(ui->scaffold);
            break;
        case OWNED_GAMES:
            nextPage = new OwnedGamesPage(ui->scaffold);
            break;
        case INSTALLED_GAMES:
        case FRIENDS:
        case CATALOG_PRODUCT_PAGE:
            nextPage = new CatalogProductPage(ui->scaffold);
            break;
        case OWNED_PRODUCT_PAGE:
            nextPage = new OwnedGamePage(ui->scaffold);
            break;
        }

        if (nextPage != nullptr) {
            QWidget *previousPage = ui->scaffoldLayout->itemAtPosition(1, 1)->widget();
            nextPage->setApiClient(apiClient);
            nextPage->initialize(data);
            connect(nextPage, &BasePage::navigateToDestination, this, &MainWindow::setDestination);
            QLayoutItem *replacedItem = ui->scaffoldLayout->replaceWidget(previousPage, nextPage);
            delete replacedItem;
            previousPage->deleteLater();
            navigationHistory.push(NavigationDestination { newPage, data });
            ui->navigateBackButton->setEnabled(navigationHistory.size() > 1);
        }
    }
}

void MainWindow::setDestination(NavigationDestination destination)
{
    navigate(destination.page, destination.parameters);
}

void MainWindow::on_discoverButton_clicked()
{
    navigate(Page::DISCOVER);
}

void MainWindow::on_recentButton_clicked()
{
    navigate(Page::RECENT);
}

void MainWindow::on_storeButton_clicked()
{
    navigate(Page::STORE);
}


void MainWindow::on_allGamesButton_clicked()
{
    navigate(Page::ALL_GAMES);
}


void MainWindow::on_wishlistButton_clicked()
{
    navigate(Page::WISHLIST);
}


void MainWindow::on_ordersButton_clicked()
{
    navigate(Page::ORDER_HISTORY);
}


void MainWindow::on_libraryButton_clicked()
{
    navigate(Page::OWNED_GAMES);
}


void MainWindow::on_installedButton_clicked()
{
    navigate(Page::INSTALLED_GAMES);
}


void MainWindow::on_friendsButton_clicked()
{
    navigate(Page::FRIENDS);
}


void MainWindow::on_settingsButton_clicked()
{
    SettingsDialog dialog(this);
    dialog.setSettingsManager(settingsManager);
    dialog.exec();
}


void MainWindow::on_actionButton_clicked()
{
    SearchDialog dialog(this);
    dialog.exec();
}

