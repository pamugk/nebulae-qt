#include "ownedgamespage.h"
#include "ui_ownedgamespage.h"

#include <QJsonDocument>
#include <QLineEdit>
#include <QMenu>
#include <QNetworkReply>
#include <QPushButton>
#include <QScrollBar>
#include <QToolButton>

#include "../api/utils/ownedproductserialization.h"
#include "../layouts/flowlayout.h"
#include "../widgets/ownedproductgriditem.h"

OwnedGamesPage::OwnedGamesPage(QWidget *parent) :
    BasePage(parent),
    ownedGamesReply(nullptr),
    ui(new Ui::OwnedGamesPage)
{
    ui->setupUi(this);

    auto popularFiltersHolder = new QWidget(this);
    auto popularFiltersHolderLayout = new QHBoxLayout();
    popularFiltersHolderLayout->setContentsMargins(0, 0, 0, 0);
    popularFiltersHolderLayout->setSpacing(0);
    popularFiltersHolder->setLayout(popularFiltersHolderLayout);
    popularFiltersHolder->layout()->addWidget(new QPushButton("Owned games", popularFiltersHolder));
#ifdef Q_OS_WINDOWS
    popularFiltersHolder->layout()->addWidget(new QPushButton("Windows", popularFiltersHolder));
#endif
#ifdef Q_OS_LINUX
    popularFiltersHolder->layout()->addWidget(new QPushButton("Linux", popularFiltersHolder));
#endif
#ifdef Q_OS_MACOS
    popularFiltersHolder->layout()->addWidget(new QPushButton("macOS", popularFiltersHolder));
#endif
    uiActions.append(popularFiltersHolder);

    auto filterToolButton = new QToolButton(this);
    filterToolButton->setIcon(QIcon(":icons/filter.svg"));
    filterToolButton->setPopupMode(QToolButton::InstantPopup);
    filterToolButton->setText("Filter");
    filterToolButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    auto filterMenu = new QMenu(filterToolButton);
    filterMenu->addMenu("Genre");
    filterMenu->addMenu("Platform")->addAction("GOG.com")->setCheckable(true);
    auto filterSubmenu = filterMenu->addMenu("OS & consoles");
    filterSubmenu->addAction("Windows")->setCheckable(true);
    filterSubmenu->addAction("macOS")->setCheckable(true);
    filterSubmenu->addAction("Linux")->setCheckable(true);
    filterMenu->addSeparator();
    filterMenu->addMenu("Rating")->addAction("No rating")->setCheckable(true);
    filterMenu->addMenu("Tags")->addAction("No tags")->setCheckable(true);
    filterSubmenu = filterMenu->addMenu("Status");
    filterSubmenu->addAction("Owned")->setCheckable(true);
    filterSubmenu->addAction("Subscriptions")->setCheckable(true);
    filterSubmenu->addSeparator();
    filterSubmenu->addAction("Installed")->setCheckable(true);
    filterSubmenu->addAction("Not installed")->setCheckable(true);
    filterSubmenu->addSeparator();
    filterSubmenu->addAction("Hidden")->setCheckable(true);
    filterToolButton->setMenu(filterMenu);
    uiActions.append(filterToolButton);

    auto addToolButton = new QToolButton(this);
    addToolButton->setIcon(QIcon(":icons/plus.svg"));
    addToolButton->setPopupMode(QToolButton::InstantPopup);
    auto addMenu = new QMenu(addToolButton);
    addMenu->addAction("Connect gaming accounts");
    addMenu->addSeparator();
    addMenu->addAction("Add game manually");
    addMenu->addAction("Redeem GOG code");
    addMenu->addAction("Scan folders for GOG games");
    addMenu->addSeparator();
    addMenu->addAction("Add friends");
    addToolButton->setMenu(addMenu);
    uiActions.append(addToolButton);

    auto searchLineEdit = new QLineEdit(this);
    searchLineEdit->setClearButtonEnabled(true);
    searchLineEdit->setPlaceholderText("Search");
    connect(searchLineEdit, &QLineEdit::textChanged, this, [this](const QString &newSearchText)
    {
        auto newSearchQuery = newSearchText.trimmed();
        if (newSearchQuery != searchQuery)
        {
            searchQuery = newSearchQuery;
            updateData();
        }
    });
    uiActions.append(searchLineEdit);

    auto viewSettingsToolButton = new QToolButton(this);
    viewSettingsToolButton->setIcon(QIcon(":icons/sliders.svg"));
    viewSettingsToolButton->setPopupMode(QToolButton::InstantPopup);

    auto viewSettingsMenu = new QMenu(viewSettingsToolButton);

    auto gridModeItem = viewSettingsMenu->addAction("Grid");
    gridModeItem->setCheckable(true);
    gridModeItem->setChecked(true);
    auto listModeItem = viewSettingsMenu->addAction("List");
    listModeItem->setCheckable(true);

    viewSettingsMenu->addSeparator();
    auto settingsSubmenu = viewSettingsMenu->addMenu("Customize grid view");
    auto gridCustomizationItem = settingsSubmenu->addAction("Title");
    gridCustomizationItem->setCheckable(true);
    gridCustomizationItem = settingsSubmenu->addAction("Rating");
    gridCustomizationItem->setCheckable(true);
    gridCustomizationItem = settingsSubmenu->addAction("Status");
    gridCustomizationItem->setCheckable(true);
    gridCustomizationItem = settingsSubmenu->addAction("Additional data");
    gridCustomizationItem->setCheckable(true);

    connect(gridModeItem, &QAction::toggled, this, [this, settingsSubmenu, listModeItem](bool toggled)
    {
        if (toggled)
        {

        }
        listModeItem->setChecked(!toggled);
        settingsSubmenu->menuAction()->setVisible(toggled);
    });

    settingsSubmenu = viewSettingsMenu->addMenu("Customize list view");
    gridCustomizationItem = settingsSubmenu->addAction("Achievements %");
    gridCustomizationItem->setCheckable(true);
    gridCustomizationItem = settingsSubmenu->addAction("Critics rating");
    gridCustomizationItem->setCheckable(true);
    gridCustomizationItem = settingsSubmenu->addAction("Game icon");
    gridCustomizationItem->setCheckable(true);
    gridCustomizationItem = settingsSubmenu->addAction("Game time");
    gridCustomizationItem->setCheckable(true);
    gridCustomizationItem = settingsSubmenu->addAction("Genres");
    gridCustomizationItem->setCheckable(true);
    gridCustomizationItem = settingsSubmenu->addAction("Last played");
    gridCustomizationItem->setCheckable(true);
    gridCustomizationItem = settingsSubmenu->addAction("Platform icon");
    gridCustomizationItem->setCheckable(true);
    gridCustomizationItem = settingsSubmenu->addAction("Platform name");
    gridCustomizationItem->setCheckable(true);
    gridCustomizationItem = settingsSubmenu->addAction("Rating");
    gridCustomizationItem->setCheckable(true);
    gridCustomizationItem = settingsSubmenu->addAction("Release date");
    gridCustomizationItem->setCheckable(true);
    gridCustomizationItem = settingsSubmenu->addAction("Size on disk");
    gridCustomizationItem->setCheckable(true);
    gridCustomizationItem = settingsSubmenu->addAction("Status");
    gridCustomizationItem->setCheckable(true);
    gridCustomizationItem = settingsSubmenu->addAction("Tags");
    gridCustomizationItem->setCheckable(true);
    settingsSubmenu->menuAction()->setVisible(false);

    connect(listModeItem, &QAction::toggled, this, [this, settingsSubmenu, gridModeItem](bool toggled)
    {
        if (toggled)
        {

        }
        gridModeItem->setChecked(!toggled);
        settingsSubmenu->menuAction()->setVisible(toggled);
    });

    viewSettingsMenu->addSeparator();

    settingsSubmenu = viewSettingsMenu->addMenu("Sort by");
    settingsSubmenu->addAction("Achievements %");
    settingsSubmenu->addAction("Critics rating");
    settingsSubmenu->addAction("Game time");
    settingsSubmenu->addAction("Genre");
    settingsSubmenu->addAction("Last played");
    settingsSubmenu->addAction("Platform");
    settingsSubmenu->addAction("Rating");
    settingsSubmenu->addAction("Release date");
    settingsSubmenu->addAction("Size on disk");
    settingsSubmenu->addAction("Tags");
    settingsSubmenu->addAction("Title");

    settingsSubmenu = viewSettingsMenu->addMenu("Group by");
    settingsSubmenu->addAction("Don't group");
    settingsSubmenu->addSeparator();
    settingsSubmenu->addAction("Critics rating");
    settingsSubmenu->addAction("Genre");
    settingsSubmenu->addAction("Installed");
    settingsSubmenu->addAction("Platform");
    settingsSubmenu->addAction("Rating");
    settingsSubmenu->addAction("Release date");
    settingsSubmenu->addAction("Subscription");
    settingsSubmenu->addAction("Tags");

    viewSettingsToolButton->setMenu(viewSettingsMenu);
    uiActions.append(viewSettingsToolButton);

    ui->contentsStack->setCurrentWidget(ui->loaderPage);
    ui->resultsStack->setCurrentWidget(ui->resultsGridPage);
    ui->resultsGridScrollAreaContents->setLayout(new FlowLayout(ui->resultsGridScrollAreaContents));
}

OwnedGamesPage::~OwnedGamesPage()
{
    for (QWidget *uiAction : std::as_const(uiActions))
    {
        delete uiAction;
    }
    uiActions.clear();
    if (ownedGamesReply != nullptr)
    {
        ownedGamesReply->abort();
    }
    delete ui;
}

const QVector<QWidget *> OwnedGamesPage::getHeaderControls()
{
    return uiActions;
}

void OwnedGamesPage::setApiClient(api::GogApiClient *apiClient)
{
    this->apiClient = apiClient;
}

void OwnedGamesPage::updateData()
{
    if (ownedGamesReply != nullptr)
    {
        ownedGamesReply->abort();
    }
    ui->contentsStack->setCurrentWidget(ui->loaderPage);
    ui->resultsGridScrollArea->verticalScrollBar()->setValue(0);
    while (!ui->resultsGridScrollAreaContents->layout()->isEmpty())
    {
        auto item = ui->resultsGridScrollAreaContents->layout()->itemAt(0);
        ui->resultsGridScrollAreaContents->layout()->removeItem(item);
        item->widget()->deleteLater();
        delete item;
    }

    ownedGamesReply = apiClient->getOwnedProducts(searchQuery);
    connect(ownedGamesReply, &QNetworkReply::finished, this, [=]()
    {
        auto networkReply = ownedGamesReply;
        ownedGamesReply = nullptr;
        if (networkReply->error() == QNetworkReply::NoError)
        {
            auto resultJson = QJsonDocument::fromJson(QString(networkReply->readAll()).toUtf8()).object();
            api::GetOwnedProductsResponse data;
            parseOwnedProductsResponse(resultJson, data);
            if (data.products.isEmpty())
            {
                ui->contentsStack->setCurrentWidget(ui->emptyPage);
            }
            else
            {
                for (const api::OwnedProduct &item : std::as_const(data.products))
                {
                    auto gridItem = new OwnedProductGridItem(item, apiClient, ui->resultsGridPage);
                    auto productId = item.id;
                    connect(gridItem, &OwnedProductGridItem::navigateToProduct, this, [this, productId]()
                    {
                        emit navigate({Page::OWNED_PRODUCT, productId});
                    });
                    ui->resultsGridScrollAreaContents->layout()->addWidget(gridItem);
                }
                ui->contentsStack->setCurrentWidget(ui->resultsPage);
            }
        }
        else if (networkReply->error() != QNetworkReply::OperationCanceledError)
        {
            ui->contentsStack->setCurrentWidget(ui->errorPage);
            qDebug() << networkReply->error() << networkReply->errorString() << QString(networkReply->readAll()).toUtf8();
        }

        networkReply->deleteLater();
    });
}

void OwnedGamesPage::initialize(const QVariant &data)
{
    updateData();
}

void OwnedGamesPage::switchUiAuthenticatedState(bool authenticated)
{

}

