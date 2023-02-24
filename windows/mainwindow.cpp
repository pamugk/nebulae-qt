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

    storePage = new StorePage(ui->pagesStack);
    allGamesPage = new AllGamesPage(ui->pagesStack);
    wishlistPage = new WishlistPage(ui->pagesStack);
    ordersPage = new OrdersPage(ui->pagesStack);

    ui->pagesStack->addWidget(storePage);
    ui->pagesStack->addWidget(allGamesPage);
    ui->pagesStack->addWidget(wishlistPage);
    ui->pagesStack->addWidget(ordersPage);

}

MainWindow::~MainWindow()
{
    delete ui;
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
}

void MainWindow::setSettingsManager(SettingsManager *settingsManager)
{
    this->settingsManager = settingsManager;
}


void MainWindow::on_discoverButton_clicked()
{

}


void MainWindow::on_recentButton_clicked()
{

}


void MainWindow::on_storeButton_clicked()
{
    ui->pagesStack->setCurrentWidget(storePage);
}


void MainWindow::on_allGamesButton_clicked()
{
    ui->pagesStack->setCurrentWidget(allGamesPage);
}


void MainWindow::on_wishlistButton_clicked()
{
    ui->pagesStack->setCurrentWidget(wishlistPage);
}


void MainWindow::on_ordersButton_clicked()
{
    ui->pagesStack->setCurrentWidget(ordersPage);
}


void MainWindow::on_libraryButton_clicked()
{

}


void MainWindow::on_installedButton_clicked()
{

}


void MainWindow::on_friendsButton_clicked()
{

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

