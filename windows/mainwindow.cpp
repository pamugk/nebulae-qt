#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include <QActionGroup>
#include <QDesktopServices>
#include <QJsonDocument>
#include <QNetworkReply>
#include <QWidgetAction>

#include "authdialog.h"
#include "legalinfodialog.h"
#include "searchdialog.h"
#include "settingsdialog.h"

#include "../api/utils/userserialization.h"
#include "../pages/allgamespage.h"
#include "../pages/catalogproductpage.h"
#include "../pages/newspage.h"
#include "../pages/orderspage.h"
#include "../pages/ownedgamepage.h"
#include "../pages/ownedgamespage.h"
#include "../pages/storepage.h"
#include "../pages/wishlistpage.h"

MainWindow::MainWindow(api::GogApiClient *apiClient,
                       SettingsManager *settingsManager,
                       QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow),
      apiClient(apiClient),
      initialized(false),
      settingsManager(settingsManager),
      userAvatarReply(nullptr),
      userReply(nullptr)
{
    ui->setupUi(this);

    connect(apiClient, &api::GogApiClient::authenticated, this, &MainWindow::switchUiAuthenticatedState);
    switchUiAuthenticatedState(apiClient->isAuthenticated());

    connect(ui->loginButton, &QPushButton::clicked, apiClient, &api::GogApiClient::grant);
    connect(apiClient, &api::GogApiClient::authorize, this, [this](const QUrl &authUrl){
        AuthDialog dialog(authUrl, this);
        dialog.exec();
    });

    auto infoMenu = new QMenu(ui->infoToolButton);
    auto addSubmenu = infoMenu->addMenu("Add games && friends");
    addSubmenu->addAction("Connect gaming accounts");
    addSubmenu->addSeparator();
    addSubmenu->addAction("Add game manually");
    addSubmenu->addAction("Redeem GOG code");
    addSubmenu->addAction("Scan folders for GOG games");
    addSubmenu->addSeparator();
    addSubmenu->addAction("Add friends");
    infoMenu->addAction("Sync integrations");
    infoMenu->addSeparator();
    connect(infoMenu->addAction("Settings"), &QAction::triggered,
            this, [this]()
    {
        SettingsDialog dialog(this);
        dialog.setSettingsManager(this->settingsManager);
        dialog.exec();
    });
    connect(infoMenu->addAction("Get support (GOG)"), &QAction::triggered,
            this, [this]()
    {
        QDesktopServices::openUrl(QUrl("https://support.gog.com/hc"));
    });
    infoMenu->addSeparator();
    connect(infoMenu->addAction("Legal && privacy"), &QAction::triggered,
            this, [this]()
    {
        LegalInfoDialog dialog(this);
        dialog.exec();
    });
    connect(infoMenu->addAction("About Qt"), &QAction::triggered,
            &QApplication::aboutQt);
    infoMenu->addSeparator();
    connect(infoMenu->addAction("Exit"), &QAction::triggered,
            this, [this]()
    {
        QApplication::instance()->exit();
    });
    ui->infoToolButton->setMenu(infoMenu);

    auto userMenu = new QMenu(ui->userToolButton);
    auto userInfoAction = new QWidgetAction(userMenu);
    auto userInfo = new QWidget(userMenu);
    auto userInfoLayout = new QGridLayout;
    auto userInfoAvatarLabel = new QLabel(userInfo);
    userInfoAvatarLabel->setMask(QRegion(QRect(0, 0, 24, 24), QRegion::Ellipse));
    userInfoAvatarLabel->setFixedSize(24, 24);
    userInfoAvatarLabel->setPixmap(QPixmap(":/icons/user.svg").scaled(userInfoAvatarLabel->size()));
    userInfoLayout->addWidget(userInfoAvatarLabel, 0, 0, 2, 1);
    userInfoLayout->addWidget(new QLabel("Loading user data…", userInfo), 0, 1);
    auto userInfoStatusLabel = new QLabel("Online", userInfo);
    userInfoLayout->addWidget(userInfoStatusLabel, 1, 1);
    userInfo->setLayout(userInfoLayout);
    userInfoAction->setDefaultWidget(userInfo);
    userMenu->addAction(userInfoAction);
    userMenu->addSeparator();

    auto userOnlineStatusGroup = new QActionGroup(userMenu);
    auto onlineAction = userMenu->addAction("Online");
    onlineAction->setCheckable(true);
    connect(onlineAction, &QAction::toggled, this, [this, userInfoStatusLabel](bool toggled)
    {
        if (toggled)
        {
            userInfoStatusLabel->setText("Online");
        }
    });
    userOnlineStatusGroup->addAction(onlineAction);
    auto offlineAction = userMenu->addAction("Offline");
    offlineAction->setCheckable(true);
    connect(offlineAction, &QAction::toggled, this, [this, userInfoStatusLabel](bool toggled)
    {
        if (toggled)
        {
            userInfoStatusLabel->setText("Offline");
        }
    });
    userOnlineStatusGroup->addAction(offlineAction);
    onlineAction->setChecked(true);
    userMenu->addSeparator();

    auto accountManagementAction = userMenu->addAction("Account management");
    connect(accountManagementAction, &QAction::triggered, this, [this]()
    {
        navigate({ Page::STORE_ACCOUNT_SETTINGS });
    });
    auto ordersHistoryAction = userMenu->addAction("Orders history");
    connect(ordersHistoryAction, &QAction::triggered, this, [this]()
    {
        navigate({ Page::ORDER_HISTORY });
    });
    userMenu->addSeparator();
    auto logoutAction = userMenu->addAction("Sign out");
    connect(logoutAction, &QAction::triggered, apiClient, &api::GogApiClient::logout);
    ui->userToolButton->setMenu(userMenu);

    NavigationDestination startDestination = NavigationDestination { Page::STORE };
    QWidget *startPage = initializePage(startDestination);
    navigationHistory.push(startDestination);
    updateCheckedDrawerDestination(startDestination.page);
    ui->scaffoldLayout->addWidget(startPage, 1, 1);
}

MainWindow::~MainWindow()
{
    if (userAvatarReply != nullptr)
    {
        userAvatarReply->abort();
    }
    if (userReply != nullptr)
    {
        userReply->abort();
    }
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
    case CATALOG_PRODUCT:
        page = new CatalogProductPage(ui->scaffold);
        break;
    case OWNED_PRODUCT:
        page = new OwnedGamePage(ui->scaffold);
        break;
    case NEWS:
        page = new NewsPage(ui->scaffold);
    case DEALS:
        break;
    case CART:
        break;
    case STORE_ACCOUNT_SETTINGS:
        break;
    }
    // TODO: remove this check after all of navigation destinations are implemented
    if (page != nullptr)
    {
        QLayoutItem *actionItem;
        while ((actionItem = ui->pageActionsHolderLayout->takeAt(0)))
        {
            actionItem->widget()->deleteLater();
            delete actionItem;
        }
        for (QWidget *actionWidget : page->getHeaderControls())
        {
            ui->pageActionsHolderLayout->addWidget(actionWidget);
        }
        connect(page, &BasePage::navigate, this, &MainWindow::navigate);
        connect(page, &BasePage::navigateBack, this, &MainWindow::navigateBack);
        page->setApiClient(apiClient);
        connect(apiClient, &api::GogApiClient::authenticated, page, &BasePage::switchUiAuthenticatedState);
        page->switchUiAuthenticatedState(apiClient->isAuthenticated());
        page->initialize(destination.parameters);
    }
    return page;
}

void MainWindow::switchUiAuthenticatedState(bool authenticated)
{
    ui->loginButton->setVisible(!authenticated);
    ui->userToolButton->setVisible(authenticated);
    if (userAvatarReply != nullptr)
    {
        userAvatarReply->abort();
    }
    if (userReply != nullptr)
    {
        userReply->abort();
    }
    if (authenticated)
    {
        userReply = apiClient->getCurrentUser();
        connect(userReply, &QNetworkReply::finished, this, [this]()
        {
            auto networkReply = userReply;
            userReply = nullptr;
            auto userInfo = static_cast<QWidgetAction *>(ui->userToolButton->menu()->actions()[0])->defaultWidget();
            auto userInfoLayout = static_cast<QGridLayout *>(userInfo->layout());
            auto userNameLabel = static_cast<QLabel *>(userInfoLayout->itemAtPosition(0, 1)->widget());
            if (networkReply->error() == QNetworkReply::NoError)
            {
                auto resultJson = QJsonDocument::fromJson(QString(networkReply->readAll()).toUtf8()).object();
                api::UserFullData data;
                parseUserFullData(resultJson, data);

                userNameLabel->setText(data.username);
                userAvatarReply = apiClient->getAnything(QString("https://images.gog.com/%1_avm.webp").arg(data.avatar.gogImageId));
                connect(userAvatarReply, &QNetworkReply::finished, this, [this]()
                {
                    auto networkReply = userAvatarReply;
                    userAvatarReply = nullptr;
                    if (networkReply->error() == QNetworkReply::NoError)
                    {
                        auto userInfo = static_cast<QWidgetAction *>(ui->userToolButton->menu()->actions()[0])->defaultWidget();
                        auto userInfoLayout = static_cast<QGridLayout *>(userInfo->layout());
                        auto userInfoAvatarLabel = static_cast<QLabel *>(userInfoLayout->itemAtPosition(0, 0)->widget());

                        QPixmap avatar;
                        avatar.loadFromData(networkReply->readAll());
                        userInfoAvatarLabel->setPixmap(avatar.scaled(userInfoAvatarLabel->size()));
                        ui->userToolButton->setIcon(QIcon(avatar.scaled(ui->userToolButton->size())));
                    }

                    networkReply->deleteLater();
                });
            }
            else if (networkReply->error() != QNetworkReply::OperationCanceledError)
            {
                userNameLabel->setText("User login not loaded");
                qDebug() << networkReply->error() << networkReply->errorString() << QString(networkReply->readAll()).toUtf8();
            }

            networkReply->deleteLater();
        });
    }
    else if (initialized)
    {
        ui->userToolButton->setIcon(QIcon(":/icons/user.svg"));
        auto userInfo = static_cast<QWidgetAction *>(ui->userToolButton->menu()->actions()[0])->defaultWidget();
        auto userInfoLayout = static_cast<QGridLayout *>(userInfo->layout());
        auto userInfoAvatarLabel = static_cast<QLabel *>(userInfoLayout->itemAtPosition(0, 0)->widget());
        userInfoAvatarLabel->setPixmap(QPixmap(":/icons/user.svg").scaled(userInfoAvatarLabel->size()));
        auto userNameLabel = static_cast<QLabel *>(userInfoLayout->itemAtPosition(0, 1)->widget());
        userNameLabel->setText("Loading user data…");
    }
    ui->galaxyLabel->setVisible(authenticated);
    ui->discoverButton->setVisible(authenticated);
    ui->recentButton->setVisible(authenticated);
    ui->wishlistButton->setVisible(authenticated);
    ui->cartButton->setVisible(authenticated);
    ui->ordersButton->setVisible(authenticated);
    ui->gamesLabel->setVisible(authenticated);
    ui->libraryButton->setVisible(authenticated);
    ui->installedButton->setVisible(authenticated);
    ui->friendsLabel->setVisible(authenticated);
    ui->friendsButton->setVisible(authenticated);
    // TODO: resolve QMenu crashes properly?
    initialized = true;
}

void MainWindow::updateCheckedDrawerDestination(Page currentPage)
{
    ui->discoverButton->setChecked(currentPage == Page::DISCOVER);
    ui->recentButton->setChecked(currentPage == Page::RECENT);
    ui->storeButton->setChecked(currentPage == Page::STORE || currentPage == Page::NEWS);
    ui->allGamesButton->setChecked(currentPage == Page::ALL_GAMES);
    ui->dealsButton->setChecked(currentPage == Page::DEALS);
    ui->wishlistButton->setChecked(currentPage == Page::WISHLIST);
    ui->cartButton->setChecked(currentPage == Page::DEALS);
    ui->ordersButton->setChecked(currentPage == Page::ORDER_HISTORY);
    ui->libraryButton->setChecked(currentPage == Page::OWNED_GAMES);
    ui->installedButton->setChecked(currentPage == Page::INSTALLED_GAMES);
    ui->friendsButton->setChecked(currentPage == Page::FRIENDS);
}

void MainWindow::navigate(NavigationDestination destination)
{
    if (navigationHistory.top() != destination)
    {
        QWidget *nextPage = initializePage(destination);
        // TODO: remove this check after all of navigation destinations are implemented
        if (nextPage == nullptr)
        {
            return;
        }
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
