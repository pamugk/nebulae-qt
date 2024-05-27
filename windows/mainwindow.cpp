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

MainWindow::MainWindow(api::GogApiClient *apiClient,
                       SettingsManager *settingsManager,
                       QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow),
      apiClient(apiClient),
      settingsManager(settingsManager)
{
    ui->setupUi(this);

    connect(apiClient, &api::GogApiClient::authenticated, this, &MainWindow::switchUiAuthenticatedState);
    switchUiAuthenticatedState(apiClient->isAuthenticated());

    connect(ui->loginButton, &QPushButton::clicked, apiClient, &api::GogApiClient::grant);
    connect(apiClient, &api::GogApiClient::authorize, this, [this](const QUrl &authUrl){
        AuthDialog dialog(authUrl, this);
        dialog.exec();
    });

    NavigationDestination startDestination = NavigationDestination { Page::STORE };
    QWidget *startPage = initializePage(startDestination);
    navigationHistory.push(startDestination);
    updateCheckedDrawerDestination(startDestination.page);
    ui->scaffoldLayout->addWidget(startPage, 1, 1);
}

MainWindow::~MainWindow()
{
    delete ui;
}

QWidget *MainWindow::initializePage(const NavigationDestination &destination)
{
    BasePage *page = nullptr;
    switch (destination.page) {
    case DISCOVER:
        break;
    case RECENT:
        break;
    case STORE:
        page = new StorePage(ui->scaffold);
        break;
    case ALL_GAMES:
        page = new AllGamesPage(ui->scaffold);
        break;
    case WISHLIST:
        page = new WishlistPage(ui->scaffold);
        break;
    case ORDER_HISTORY:
        page = new OrdersPage(ui->scaffold);
        break;
    case OWNED_GAMES:
        page = new OwnedGamesPage(ui->scaffold);
        break;
    case INSTALLED_GAMES:
        break;
    case FRIENDS:
        break;
    case CATALOG_PRODUCT_PAGE:
        page = new CatalogProductPage(ui->scaffold);
        break;
    case OWNED_PRODUCT_PAGE:
        page = new OwnedGamePage(ui->scaffold);
        break;
    }
    connect(page, &BasePage::navigate, this, &MainWindow::navigate);
    connect(page, &BasePage::navigateBack, this, &MainWindow::navigateBack);
    connect(apiClient, &api::GogApiClient::authenticated, page, &BasePage::switchUiAuthenticatedState);
    page->switchUiAuthenticatedState(apiClient->isAuthenticated());
    page->setApiClient(apiClient);
    page->initialize(destination.parameters);
    return page;
}

void MainWindow::switchUiAuthenticatedState(bool authenticated)
{
    ui->loginButton->setVisible(!authenticated);
    ui->galaxyLabel->setVisible(authenticated);
    ui->discoverButton->setVisible(authenticated);
    ui->recentButton->setVisible(authenticated);
    ui->wishlistButton->setVisible(authenticated);
    ui->ordersButton->setVisible(authenticated);
    ui->gamesLabel->setVisible(authenticated);
    ui->libraryButton->setVisible(authenticated);
    ui->installedButton->setVisible(authenticated);
    ui->friendsLabel->setVisible(authenticated);
    ui->friendsButton->setVisible(authenticated);
}

void MainWindow::updateCheckedDrawerDestination(Page currentPage)
{
    ui->discoverButton->setChecked(currentPage == Page::DISCOVER);
    ui->recentButton->setChecked(currentPage == Page::RECENT);
    ui->storeButton->setChecked(currentPage == Page::STORE || currentPage == Page::CATALOG_PRODUCT_PAGE);
    ui->wishlistButton->setChecked(currentPage == Page::WISHLIST);
    ui->ordersButton->setChecked(currentPage == Page::ORDER_HISTORY);
    ui->libraryButton->setChecked(currentPage == Page::OWNED_GAMES || currentPage == Page::OWNED_PRODUCT_PAGE);
    ui->installedButton->setChecked(currentPage == Page::INSTALLED_GAMES);
    ui->friendsButton->setChecked(currentPage == Page::FRIENDS);
}

void MainWindow::navigate(NavigationDestination destination)
{
    if (navigationHistory.top().page != destination.page)
    {
        QWidget *nextPage = initializePage(destination);
        QWidget *previousPage = ui->scaffoldLayout->itemAtPosition(1, 1)->widget();
        QLayoutItem *replacedItem = ui->scaffoldLayout->replaceWidget(previousPage, nextPage);
        delete replacedItem;
        previousPage->deleteLater();

        navigationHistory.push(destination);
        navigationHistoryReplay.clear();
        ui->navigateBackButton->setEnabled(true);
        ui->navigateForwardButton->setEnabled(false);
        updateCheckedDrawerDestination(destination.page);
    }
}

void MainWindow::navigateBack()
{
    const NavigationDestination poppedDestination = navigationHistory.pop();
    QWidget *currentPage = initializePage(navigationHistory.top());
    QWidget *replacedPage = ui->scaffoldLayout->itemAtPosition(1, 1)->widget();
    QLayoutItem *replacedItem = ui->scaffoldLayout->replaceWidget(replacedPage, currentPage);
    delete replacedItem;
    replacedPage->deleteLater();

    navigationHistoryReplay.push(poppedDestination);
    ui->navigateBackButton->setEnabled(navigationHistory.size() > 1);
    ui->navigateForwardButton->setEnabled(true);
    updateCheckedDrawerDestination(navigationHistory.top().page);
}

void MainWindow::navigateForward()
{
    const NavigationDestination pushedDestination = navigationHistoryReplay.pop();
    QWidget *currentPage = initializePage(pushedDestination);
    QWidget *replacedPage = ui->scaffoldLayout->itemAtPosition(1, 1)->widget();
    QLayoutItem *replacedItem = ui->scaffoldLayout->replaceWidget(replacedPage, currentPage);
    delete replacedItem;
    replacedPage->deleteLater();

    navigationHistory.push(pushedDestination);
    ui->navigateBackButton->setEnabled(true);
    ui->navigateForwardButton->setEnabled(navigationHistoryReplay.size() > 0);
    updateCheckedDrawerDestination(pushedDestination.page);
}

void MainWindow::on_discoverButton_clicked()
{
    navigate(NavigationDestination { Page::DISCOVER });
}

void MainWindow::on_recentButton_clicked()
{
    navigate(NavigationDestination { Page::RECENT });
}

void MainWindow::on_storeButton_clicked()
{
    navigate(NavigationDestination { Page::STORE });
}


void MainWindow::on_allGamesButton_clicked()
{
    navigate(NavigationDestination { Page::ALL_GAMES });
}


void MainWindow::on_wishlistButton_clicked()
{
    navigate(NavigationDestination { Page::WISHLIST });
}


void MainWindow::on_ordersButton_clicked()
{
    navigate(NavigationDestination { Page::ORDER_HISTORY });
}


void MainWindow::on_libraryButton_clicked()
{
    navigate(NavigationDestination { Page::OWNED_GAMES });
}


void MainWindow::on_installedButton_clicked()
{
    navigate(NavigationDestination { Page::INSTALLED_GAMES });
}


void MainWindow::on_friendsButton_clicked()
{
    navigate(NavigationDestination { Page::FRIENDS });
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


void MainWindow::on_navigateBackButton_clicked()
{
    navigateBack();
}


void MainWindow::on_navigateForwardButton_clicked()
{
    navigateForward();
}

