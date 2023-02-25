#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include "../pages/allgamespage.h"
#include "../pages/orderspage.h"
#include "../pages/storepage.h"
#include "../pages/wishlistpage.h"

#include "authdialog.h"
#include "searchdialog.h"
#include "settingsdialog.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow),
      apiClient(nullptr),
      settingsManager(nullptr)
{
    ui->setupUi(this);

    pages[Page::DISCOVER] = new StorePage(ui->pagesStack);
    pages[Page::ALL_GAMES] = new AllGamesPage(ui->pagesStack);
    pages[Page::WISHLIST] = new WishlistPage(ui->pagesStack);
    pages[Page::ORDER_HISTORY] = new OrdersPage(ui->pagesStack);

    BasePage *item;
    foreach (item, pages.values())
    {
        ui->pagesStack->addWidget(item);
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::navigate(Page newPage)
{
    if (pages.contains(newPage))
    {
        auto page = pages[newPage];
        ui->pagesStack->setCurrentWidget(page);
        page->initialize();
        if (pages.contains(currentPage))
        {
            pages[currentPage]->clear();
        }
        currentPage = newPage;
    }
}

void MainWindow::setApiClient(GogApiClient *apiClient)
{
    this->apiClient = apiClient;
    ui->loginButton->setVisible(!apiClient->isAuthenticated());
    connect(apiClient, &GogApiClient::authenticated, this, [this]{
        ui->loginButton->setVisible(false);
    });
    connect(ui->loginButton, &QPushButton::clicked, apiClient, &GogApiClient::grant);
    connect(apiClient, &GogApiClient::authorize, this, [this](const QUrl &authUrl){
        AuthDialog dialog(this);
        dialog.setUrl(authUrl);
        dialog.exec();
    });

    BasePage *item;
    foreach (item, pages.values())
    {
        item->setApiClient(apiClient);
    }
}

void MainWindow::setSettingsManager(SettingsManager *settingsManager)
{
    this->settingsManager = settingsManager;
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

