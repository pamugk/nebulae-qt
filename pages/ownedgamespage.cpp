#include "ownedgamespage.h"
#include "ui_ownedgamespage.h"

#include <QActionGroup>
#include <QLineEdit>
#include <QListWidgetItem>
#include <QMenu>
#include <QPushButton>
#include <QScrollBar>
#include <QSlider>
#include <QToolButton>
#include <QWidgetAction>

#include "../db/database.h"
#include "../layouts/flowlayout.h"
#include "../widgets/ownedproductgriditem.h"

OwnedGamesPage::OwnedGamesPage(QWidget *parent) :
    BasePage(parent),
    currentGridImageSize(1),
    currentListIconSize(0),
    gridImageSizes({ QSize(110, 155), QSize(120, 170), QSize(150, 210), QSize(185, 260), QSize(190, 265), QSize(260, 365) }),
    gridLayout(true),
    listIconSizes({ QSize(16, 16), QSize(32, 32), QSize(48, 48) }),
    ui(new Ui::OwnedGamesPage)
{
    // TODO: restore filters state from disk
    request.hidden = false;
    request.owned = true;
    request.order = api::UserReleaseOrder::TITLE;
    request.grouping = api::UserReleaseGrouping::NO_GROUP;

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
    uiActions << popularFiltersHolder;

    auto filterToolButton = new QToolButton(this);
    filterToolButton->setIcon(QIcon(":icons/filter.svg"));
    filterToolButton->setPopupMode(QToolButton::InstantPopup);
    filterToolButton->setText("Filter");
    filterToolButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    auto filterMenu = new QMenu(filterToolButton);
    filterMenu->addMenu("Genre");
    filterMenu->addMenu("Platform");
    filterMenu->addMenu("OS && consoles");
    filterMenu->addMenu("Rating");
    filterMenu->addMenu("Tags");
    auto filterSubmenu = filterMenu->addMenu("Status");
    auto filterItem = filterSubmenu->addAction("Owned");
    filterItem->setCheckable(true);
    filterItem->setChecked(request.owned);
    connect(filterItem, &QAction::toggled, this, [this](bool toggled)
    {
       request.owned = toggled;
       updateData();
    });
    filterItem = filterSubmenu->addAction("Subscriptions");
    filterItem->setCheckable(true);
    filterSubmenu->addSeparator();
    filterItem = filterSubmenu->addAction("Installed");
    filterItem->setCheckable(true);
    filterItem = filterSubmenu->addAction("Not installed");
    filterItem->setCheckable(true);
    filterSubmenu->addSeparator();
    filterItem = filterSubmenu->addAction("Hidden");
    filterItem->setCheckable(true);
    connect(filterItem, &QAction::toggled, this, [this](bool toggled)
    {
       request.hidden = toggled;
       updateData();
    });
    filterToolButton->setMenu(filterMenu);
    uiActions << filterToolButton;

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
    uiActions << addToolButton;

    auto searchLineEdit = new QLineEdit(this);
    searchLineEdit->setClearButtonEnabled(true);
    searchLineEdit->setPlaceholderText("Search");
    connect(searchLineEdit, &QLineEdit::textChanged, this, [this](const QString &newSearchText)
    {
        auto newSearchQuery = newSearchText.trimmed();
        if (newSearchQuery != request.query)
        {
            request.query = newSearchQuery;
            updateData();
        }
    });
    uiActions << searchLineEdit;

    auto viewSettingsToolButton = new QToolButton(this);
    viewSettingsToolButton->setIcon(QIcon(":icons/sliders.svg"));
    viewSettingsToolButton->setPopupMode(QToolButton::InstantPopup);

    auto viewSettingsMenu = new QMenu(viewSettingsToolButton);

    auto resultsLayoutGroup = new QActionGroup(viewSettingsMenu);
    auto gridModeItem = viewSettingsMenu->addAction("Grid");
    gridModeItem->setCheckable(true);
    gridModeItem->setChecked(gridLayout);
    connect(gridModeItem, &QAction::toggled, this, [this](bool toggled)
    {
        if (toggled)
        {
            gridLayout = true;
            layoutData();
        }
    });
    resultsLayoutGroup->addAction(gridModeItem);
    auto listModeItem = viewSettingsMenu->addAction("List");
    listModeItem->setCheckable(true);
    listModeItem->setChecked(!gridLayout);
    viewSettingsMenu->addAction(listModeItem);
    connect(listModeItem, &QAction::toggled, this, [this](bool toggled)
    {
        if (toggled)
        {
            gridLayout = false;
            layoutData();
        }
    });
    resultsLayoutGroup->addAction(listModeItem);
    viewSettingsMenu->addSeparator();

    auto gridImageScaleSlider = new QSlider(Qt::Horizontal);
    gridImageScaleSlider->setMinimum(0);
    gridImageScaleSlider->setMaximum(5);
    gridImageScaleSlider->setValue(currentGridImageSize);
    connect(gridImageScaleSlider, &QSlider::valueChanged, this, [this](int newValue)
    {
        currentGridImageSize = newValue;
        emit gridItemImageSizeChanged(gridImageSizes[newValue]);
    });

    auto gridSettingsSubmenu = viewSettingsMenu->addMenu("Customize grid view");
    auto gridImageScaleAction = new QWidgetAction(gridSettingsSubmenu);
    gridImageScaleAction->setDefaultWidget(gridImageScaleSlider);
    gridSettingsSubmenu->addAction(gridImageScaleAction);
    gridSettingsSubmenu->addSeparator();
    auto gridCustomizationItem = gridSettingsSubmenu->addAction("Title");
    gridCustomizationItem->setCheckable(true);
    connect(gridCustomizationItem, &QAction::toggled, this, &OwnedGamesPage::gridItemTitleVisibilityChanged);
    gridCustomizationItem = gridSettingsSubmenu->addAction("Rating");
    connect(gridCustomizationItem, &QAction::toggled, this, &OwnedGamesPage::gridItemRatingVisibilityChanged);
    gridCustomizationItem->setCheckable(true);
    gridCustomizationItem = gridSettingsSubmenu->addAction("Status");
    connect(gridCustomizationItem, &QAction::toggled, this, &OwnedGamesPage::gridItemStatusVisibilityChanged);
    gridCustomizationItem->setCheckable(true);
    gridCustomizationItem = gridSettingsSubmenu->addAction("Additional data");
    connect(gridCustomizationItem, &QAction::toggled, this, &OwnedGamesPage::gridItemAdditionalDataVisibilityChanged);
    gridCustomizationItem->setCheckable(true);
    gridCustomizationItem = gridSettingsSubmenu->addSeparator();
    gridCustomizationItem->setVisible(false);
    connect(this, &OwnedGamesPage::gridItemAdditionalDataVisibilityChanged, gridCustomizationItem, &QAction::setVisible);
    auto gridAdditionalCustomizationGroup = new QActionGroup(gridSettingsSubmenu);
    gridCustomizationItem = gridSettingsSubmenu->addAction("Company");
    gridCustomizationItem->setCheckable(true);
    gridCustomizationItem->setVisible(false);
    gridAdditionalCustomizationGroup->addAction(gridCustomizationItem);
    connect(this, &OwnedGamesPage::gridItemAdditionalDataVisibilityChanged, gridCustomizationItem, &QAction::setVisible);
    connect(gridCustomizationItem, &QAction::toggled, this, [this](bool checked)
    {
       if (checked)
       {
           emit gridItemAdditionalDataDisplayed(OwnedProductGridItem::AdditionalInfo::COMPANY);
       }
    });
    gridCustomizationItem = gridSettingsSubmenu->addAction("Genres");
    gridCustomizationItem->setCheckable(true);
    gridCustomizationItem->setVisible(false);
    gridAdditionalCustomizationGroup->addAction(gridCustomizationItem);
    connect(this, &OwnedGamesPage::gridItemAdditionalDataVisibilityChanged, gridCustomizationItem, &QAction::setVisible);
    connect(gridCustomizationItem, &QAction::toggled, this, [this](bool checked)
    {
       if (checked)
       {
           emit gridItemAdditionalDataDisplayed(OwnedProductGridItem::AdditionalInfo::GENRES);
       }
    });
    gridCustomizationItem = gridSettingsSubmenu->addAction("My stats");
    gridCustomizationItem->setCheckable(true);
    gridCustomizationItem->setVisible(false);
    gridAdditionalCustomizationGroup->addAction(gridCustomizationItem);
    connect(this, &OwnedGamesPage::gridItemAdditionalDataVisibilityChanged, gridCustomizationItem, &QAction::setVisible);
    connect(gridCustomizationItem, &QAction::toggled, this, [this](bool checked)
    {
       if (checked)
       {
           emit gridItemAdditionalDataDisplayed(OwnedProductGridItem::AdditionalInfo::STATS);
       }
    });
    gridCustomizationItem = gridSettingsSubmenu->addAction("My tags");
    gridCustomizationItem->setCheckable(true);
    gridCustomizationItem->setVisible(false);
    gridAdditionalCustomizationGroup->addAction(gridCustomizationItem);
    connect(this, &OwnedGamesPage::gridItemAdditionalDataVisibilityChanged, gridCustomizationItem, &QAction::setVisible);
    connect(gridCustomizationItem, &QAction::toggled, this, [this](bool checked)
    {
       if (checked)
       {
           emit gridItemAdditionalDataDisplayed(OwnedProductGridItem::AdditionalInfo::TAGS);
       }
    });
    gridCustomizationItem = gridSettingsSubmenu->addAction("Platform");
    gridCustomizationItem->setCheckable(true);
    gridCustomizationItem->setVisible(false);
    gridAdditionalCustomizationGroup->addAction(gridCustomizationItem);
    gridCustomizationItem->setChecked(true);
    connect(this, &OwnedGamesPage::gridItemAdditionalDataVisibilityChanged, gridCustomizationItem, &QAction::setVisible);
    connect(gridCustomizationItem, &QAction::toggled, this, [this](bool checked)
    {
       if (checked)
       {
           emit gridItemAdditionalDataDisplayed(OwnedProductGridItem::AdditionalInfo::PLATFORM);
       }
    });
    gridSettingsSubmenu->menuAction()->setVisible(gridModeItem->isChecked());
    connect(gridModeItem, &QAction::toggled, gridSettingsSubmenu->menuAction(), &QAction::setVisible);

    auto listIconScaleSlider = new QSlider(Qt::Horizontal);
    listIconScaleSlider->setMinimum(0);
    listIconScaleSlider->setMaximum(2);
    listIconScaleSlider->setValue(currentListIconSize);
    connect(listIconScaleSlider, &QSlider::valueChanged, this, [this](int newValue)
    {
        currentListIconSize = newValue;
        ui->resultsTree->setIconSize(listIconSizes[currentListIconSize]);
    });
    ui->resultsTree->setIconSize(listIconSizes[currentListIconSize]);

    auto listSettingsSubmenu = viewSettingsMenu->addMenu("Customize list view");
    auto listIconScaleAction = new QWidgetAction(listSettingsSubmenu);
    listIconScaleAction->setDefaultWidget(listIconScaleSlider);
    listSettingsSubmenu->addAction(listIconScaleAction);
    listSettingsSubmenu->addSeparator();
    auto listCustomizationItem = listSettingsSubmenu->addAction("Achievements %");
    listCustomizationItem->setCheckable(true);
    listCustomizationItem->setChecked(true);
    connect(listCustomizationItem, &QAction::toggled, this, [this](bool toggled)
    {
        ui->resultsTree->setColumnHidden(10, !toggled);
    });
    listCustomizationItem = listSettingsSubmenu->addAction("Critics rating");
    listCustomizationItem->setCheckable(true);
    listCustomizationItem->setChecked(true);
    connect(listCustomizationItem, &QAction::toggled, this, [this](bool toggled)
    {
        ui->resultsTree->setColumnHidden(8, !toggled);
    });
    listCustomizationItem = listSettingsSubmenu->addAction("Game icon");
    listCustomizationItem->setCheckable(true);
    listCustomizationItem->setChecked(true);
    connect(listCustomizationItem, &QAction::toggled, this, [this, listIconScaleSlider](bool toggled)
    {
        ui->resultsTree->setIconSize(toggled ? listIconSizes[currentListIconSize] : QSize(0, 0));
        listIconScaleSlider->setEnabled(toggled);
    });
    listCustomizationItem = listSettingsSubmenu->addAction("Game time");
    listCustomizationItem->setCheckable(true);
    listCustomizationItem->setChecked(true);
    connect(listCustomizationItem, &QAction::toggled, this, [this](bool toggled)
    {
        ui->resultsTree->setColumnHidden(7, !toggled);
    });
    listCustomizationItem = listSettingsSubmenu->addAction("Genres");
    listCustomizationItem->setCheckable(true);
    listCustomizationItem->setChecked(true);
    connect(listCustomizationItem, &QAction::toggled, this, [this](bool toggled)
    {
        ui->resultsTree->setColumnHidden(9, !toggled);
    });
    listCustomizationItem = listSettingsSubmenu->addAction("Last played");
    listCustomizationItem->setCheckable(true);
    listCustomizationItem->setChecked(true);
    connect(listCustomizationItem, &QAction::toggled, this, [this](bool toggled)
    {
        ui->resultsTree->setColumnHidden(6, !toggled);
    });
    listCustomizationItem = listSettingsSubmenu->addAction("Platform icon");
    listCustomizationItem->setCheckable(true);
    listCustomizationItem->setVisible(false);
    connect(listCustomizationItem, &QAction::toggled, this, [this](bool toggled)
    {
    });
    listCustomizationItem = listSettingsSubmenu->addAction("Platform name");
    listCustomizationItem->setCheckable(true);
    listCustomizationItem->setChecked(true);
    connect(listCustomizationItem, &QAction::toggled, this, [this](bool toggled)
    {
        ui->resultsTree->setColumnHidden(2, !toggled);
    });
    listCustomizationItem = listSettingsSubmenu->addAction("Rating");
    listCustomizationItem->setCheckable(true);
    listCustomizationItem->setChecked(true);
    connect(listCustomizationItem, &QAction::toggled, this, [this](bool toggled)
    {
        ui->resultsTree->setColumnHidden(1, !toggled);
    });
    listCustomizationItem = listSettingsSubmenu->addAction("Release date");
    listCustomizationItem->setCheckable(true);
    listCustomizationItem->setChecked(true);
    connect(listCustomizationItem, &QAction::toggled, this, [this](bool toggled)
    {
        ui->resultsTree->setColumnHidden(4, !toggled);
    });
    listCustomizationItem = listSettingsSubmenu->addAction("Size on disk");
    listCustomizationItem->setCheckable(true);
    listCustomizationItem->setChecked(false);
    connect(listCustomizationItem, &QAction::toggled, this, [this](bool toggled)
    {
        ui->resultsTree->setColumnHidden(5, !toggled);
    });
    ui->resultsTree->setColumnHidden(5, true);
    listCustomizationItem = listSettingsSubmenu->addAction("Status");
    listCustomizationItem->setCheckable(true);
    listCustomizationItem->setVisible(false);
    connect(listCustomizationItem, &QAction::toggled, this, [this](bool toggled)
    {
    });
    listCustomizationItem = listSettingsSubmenu->addAction("Tags");
    listCustomizationItem->setCheckable(true);
    listCustomizationItem->setChecked(true);
    connect(listCustomizationItem, &QAction::toggled, this, [this](bool toggled)
    {
        ui->resultsTree->setColumnHidden(3, !toggled);
    });
    listSettingsSubmenu->menuAction()->setVisible(listModeItem->isChecked());
    connect(listModeItem, &QAction::toggled, listSettingsSubmenu->menuAction(), &QAction::setVisible);

    viewSettingsMenu->addSeparator();

    auto settingsSubmenu = viewSettingsMenu->addMenu("Sort by");
    auto sortOrderGroup = new QActionGroup(settingsSubmenu);
    auto sortItem = settingsSubmenu->addAction("Achievements %");
    sortItem->setCheckable(true);
    sortItem->setChecked(request.order == api::UserReleaseOrder::ACHIEVEMENTS);
    connect(sortItem, &QAction::toggled, this, [this](bool toggled)
    {
       if (toggled)
       {
           request.order = api::UserReleaseOrder::ACHIEVEMENTS;
           updateData();
       }
    });
    sortOrderGroup->addAction(sortItem);
    sortItem = settingsSubmenu->addAction("Critics rating");
    sortItem->setCheckable(true);
    sortItem->setChecked(request.order == api::UserReleaseOrder::CRITICS_RATING);
    connect(sortItem, &QAction::toggled, this, [this](bool toggled)
    {
       if (toggled)
       {
           request.order = api::UserReleaseOrder::CRITICS_RATING;
           updateData();
       }
    });
    sortOrderGroup->addAction(sortItem);
    sortItem = settingsSubmenu->addAction("Game time");
    sortItem->setCheckable(true);
    sortItem->setChecked(request.order == api::UserReleaseOrder::GAME_TIME);
    connect(sortItem, &QAction::toggled, this, [this](bool toggled)
    {
       if (toggled)
       {
           request.order = api::UserReleaseOrder::GAME_TIME;
           updateData();
       }
    });
    sortOrderGroup->addAction(sortItem);
    sortItem = settingsSubmenu->addAction("Genre");
    sortItem->setCheckable(true);
    sortItem->setChecked(request.order == api::UserReleaseOrder::GENRE);
    connect(sortItem, &QAction::toggled, this, [this](bool toggled)
    {
       if (toggled)
       {
           request.order = api::UserReleaseOrder::GENRE;
           updateData();
       }
    });
    sortOrderGroup->addAction(sortItem);
    sortItem = settingsSubmenu->addAction("Last played");
    sortItem->setCheckable(true);
    sortItem->setChecked(request.order == api::UserReleaseOrder::LAST_PLAYED);
    connect(sortItem, &QAction::toggled, this, [this](bool toggled)
    {
       if (toggled)
       {
           request.order = api::UserReleaseOrder::LAST_PLAYED;
           updateData();
       }
    });
    sortOrderGroup->addAction(sortItem);
    sortItem = settingsSubmenu->addAction("Platform");
    sortItem->setCheckable(true);
    sortItem->setChecked(request.order == api::UserReleaseOrder::PLATFORM);
    connect(sortItem, &QAction::toggled, this, [this](bool toggled)
    {
       if (toggled)
       {
           request.order = api::UserReleaseOrder::PLATFORM;
           updateData();
       }
    });
    sortOrderGroup->addAction(sortItem);
    sortItem = settingsSubmenu->addAction("Rating");
    sortItem->setCheckable(true);
    sortItem->setChecked(request.order == api::UserReleaseOrder::RATING);
    connect(sortItem, &QAction::toggled, this, [this](bool toggled)
    {
       if (toggled)
       {
           request.order = api::UserReleaseOrder::RATING;
           updateData();
       }
    });
    sortOrderGroup->addAction(sortItem);
    sortItem = settingsSubmenu->addAction("Release date");
    sortItem->setCheckable(true);
    sortItem->setChecked(request.order == api::UserReleaseOrder::RELEASE_DATE);
    connect(sortItem, &QAction::toggled, this, [this](bool toggled)
    {
       if (toggled)
       {
           request.order = api::UserReleaseOrder::RELEASE_DATE;
           updateData();
       }
    });
    sortOrderGroup->addAction(sortItem);
    sortItem = settingsSubmenu->addAction("Size on disk");
    sortItem->setCheckable(true);
    sortItem->setChecked(request.order == api::UserReleaseOrder::SIZE_ON_DISK);
    connect(sortItem, &QAction::toggled, this, [this](bool toggled)
    {
       if (toggled)
       {
           request.order = api::UserReleaseOrder::SIZE_ON_DISK;
           updateData();
       }
    });
    sortOrderGroup->addAction(sortItem);
    sortItem = settingsSubmenu->addAction("Tags");
    sortItem->setCheckable(true);
    sortItem->setChecked(request.order == api::UserReleaseOrder::TAG);
    connect(sortItem, &QAction::toggled, this, [this](bool toggled)
    {
       if (toggled)
       {
           request.order = api::UserReleaseOrder::TAG;
           updateData();
       }
    });
    sortOrderGroup->addAction(sortItem);
    sortItem = settingsSubmenu->addAction("Title");
    sortItem->setCheckable(true);
    sortItem->setChecked(request.order == api::UserReleaseOrder::TITLE);
    connect(sortItem, &QAction::toggled, this, [this](bool toggled)
    {
       if (toggled)
       {
           request.order = api::UserReleaseOrder::TITLE;
           updateData();
       }
    });
    sortOrderGroup->addAction(sortItem);

    settingsSubmenu = viewSettingsMenu->addMenu("Group by");
    auto groupingGroup = new QActionGroup(settingsSubmenu);
    auto groupingAction = settingsSubmenu->addAction("Don't group");
    groupingAction->setCheckable(true);
    groupingAction->setChecked(request.grouping == api::UserReleaseGrouping::NO_GROUP);
    connect(groupingAction, &QAction::toggled, this, [this](bool toggled)
    {
       if (toggled)
       {
           request.grouping = api::UserReleaseGrouping::NO_GROUP;
           updateData();
       }
    });
    groupingGroup->addAction(groupingAction);
    settingsSubmenu->addSeparator();
    groupingAction = settingsSubmenu->addAction("Critics rating");
    groupingAction->setCheckable(true);
    groupingAction->setChecked(request.grouping == api::UserReleaseGrouping::CRITICS_RATING_GROUP);
    connect(groupingAction, &QAction::toggled, this, [this](bool toggled)
    {
       if (toggled)
       {
           request.grouping = api::UserReleaseGrouping::CRITICS_RATING_GROUP;
           updateData();
       }
    });
    groupingGroup->addAction(groupingAction);
    groupingAction = settingsSubmenu->addAction("Genre");
    groupingAction->setCheckable(true);
    groupingAction->setChecked(request.grouping == api::UserReleaseGrouping::GENRE_GROUP);
    connect(groupingAction, &QAction::toggled, this, [this](bool toggled)
    {
       if (toggled)
       {
           request.grouping = api::UserReleaseGrouping::GENRE_GROUP;
           updateData();
       }
    });
    groupingAction->setVisible(false);
    groupingGroup->addAction(groupingAction);
    groupingAction = settingsSubmenu->addAction("Installed");
    groupingAction->setCheckable(true);
    groupingAction->setChecked(request.grouping == api::UserReleaseGrouping::INSTALLED_GROUP);
    connect(groupingAction, &QAction::toggled, this, [this](bool toggled)
    {
       if (toggled)
       {
           request.grouping = api::UserReleaseGrouping::INSTALLED_GROUP;
           updateData();
       }
    });
    groupingGroup->addAction(groupingAction);
    groupingAction = settingsSubmenu->addAction("Platform");
    groupingAction->setCheckable(true);
    groupingAction->setChecked(request.grouping == api::UserReleaseGrouping::PLATFORM_GROUP);
    connect(groupingAction, &QAction::toggled, this, [this](bool toggled)
    {
       if (toggled)
       {
           request.grouping = api::UserReleaseGrouping::PLATFORM_GROUP;
           updateData();
       }
    });
    groupingGroup->addAction(groupingAction);
    groupingAction = settingsSubmenu->addAction("Rating");
    groupingAction->setCheckable(true);
    groupingAction->setChecked(request.grouping == api::UserReleaseGrouping::RATING_GROUP);
    connect(groupingAction, &QAction::toggled, this, [this](bool toggled)
    {
       if (toggled)
       {
           request.grouping = api::UserReleaseGrouping::RATING_GROUP;
           updateData();
       }
    });
    groupingGroup->addAction(groupingAction);
    groupingAction = settingsSubmenu->addAction("Release date");
    groupingAction->setCheckable(true);
    groupingAction->setChecked(request.grouping == api::UserReleaseGrouping::RELEASE_DATE_GROUP);
    connect(groupingAction, &QAction::toggled, this, [this](bool toggled)
    {
       if (toggled)
       {
           request.grouping = api::UserReleaseGrouping::RELEASE_DATE_GROUP;
           updateData();
       }
    });
    groupingGroup->addAction(groupingAction);
    groupingAction = settingsSubmenu->addAction("Subscription");
    groupingAction->setCheckable(true);
    groupingAction->setChecked(request.grouping == api::UserReleaseGrouping::SUBSCRIPTION_GROUP);
    connect(groupingAction, &QAction::toggled, this, [this](bool toggled)
    {
       if (toggled)
       {
           request.grouping = api::UserReleaseGrouping::SUBSCRIPTION_GROUP;
           updateData();
       }
    });
    groupingGroup->addAction(groupingAction);
    groupingAction = settingsSubmenu->addAction("Tags");
    groupingAction->setCheckable(true);
    groupingAction->setChecked(request.grouping == api::UserReleaseGrouping::TAG_GROUP);
    connect(groupingAction, &QAction::toggled, this, [this](bool toggled)
    {
       if (toggled)
       {
           request.grouping = api::UserReleaseGrouping::TAG_GROUP;
           updateData();
       }
    });
    groupingAction->setVisible(false);
    groupingGroup->addAction(groupingAction);

    viewSettingsToolButton->setMenu(viewSettingsMenu);
    uiActions << viewSettingsToolButton;

    ui->contentsStack->setCurrentWidget(ui->loaderPage);
    ui->resultsStack->setCurrentWidget(ui->resultsGridPage);
    connect(ui->resultsTree, &QTreeWidget::itemClicked, this, [this](QTreeWidgetItem *item)
    {
        int clickedReleaseIndex;
        int groupIndex;
        if (request.grouping == api::UserReleaseGrouping::NO_GROUP)
        {
            clickedReleaseIndex = item->treeWidget()->indexOfTopLevelItem(item);
            groupIndex = 0;
        }
        else if (item->parent() == nullptr)
        {
            return;
        }
        else
        {
            auto groupItem = item->parent();
            clickedReleaseIndex = groupItem->indexOfChild(item);
            groupIndex = item->treeWidget()->indexOfTopLevelItem(groupItem);
        }

        if (clickedReleaseIndex == -1)
        {
            qDebug() << "Unknown release item (?)";
        }
        else if (groupIndex == -1)
        {
            qDebug() << "Unknown group item (?)";
        }
        else
        {
            emit navigate({ RELEASE, data[groupIndex].items[clickedReleaseIndex].id });
        }
    });
}

OwnedGamesPage::~OwnedGamesPage()
{
    for (auto iconReply : std::as_const(listIconReplies))
    {
        if (iconReply != nullptr)
        {
            iconReply->abort();
        }
    }
    listIconReplies.clear();
    uiActions.clear();
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

QString buildGroupTitle(const db::UserReleaseGroup &group,
                        api::UserReleaseGrouping grouping,
                        int currentYear)
{
    QString groupTitle;
    switch (grouping)
    {
    case api::NO_GROUP:
        break;
    case api::CRITICS_RATING_GROUP:
        if (group.discriminator.isNull())
        {
            groupTitle = "NO RATING";
        }
        else
        {
            QVariantMap interval = group.discriminator.toMap();
            groupTitle = QString("%1 - %2").arg(interval["start"].toString(), interval["end"].toString());
        }
        break;
    case api::GENRE_GROUP:
        break;
    case api::INSTALLED_GROUP:
        groupTitle = group.discriminator.toBool() ? "INSTALLED" : "NOT INSTALLED";
        break;
    case api::PLATFORM_GROUP:
        groupTitle = group.discriminator.toString() == "gog" ? "GOG.COM" : "OTHER";
        break;
    case api::RATING_GROUP:
        if (group.discriminator.isNull())
        {
            groupTitle = "NO RATING";
        }
        else
        {
            int rating = group.discriminator.toInt();
            groupTitle = QString(rating, u'★') + QString(5 - rating, u'☆');
        }
        break;
    case api::RELEASE_DATE_GROUP:
        if (group.discriminator.isNull())
        {
            groupTitle = "UNKNOWN RELEASE DATE";
        }
        else
        {
            QVariantMap interval = group.discriminator.toMap();
            if (interval.isEmpty())
            {
                int groupYear = group.discriminator.toInt();
                groupTitle = groupYear == currentYear ? "THIS YEAR" : group.discriminator.toString();
            }
            else
            {
                groupTitle = QString("%1 - %2").arg(interval["start"].toString(), interval["end"].toString());
            }
        }
        break;
    case api::SUBSCRIPTION_GROUP:
        groupTitle = group.discriminator.toBool() ? "IN SUBSCRIPTIONS" : "NOT IN SUBSCRIPTIONS";
    case api::TAG_GROUP:
        break;
    }
    return groupTitle;
}

void initializeUserReleaseRow(QTreeWidgetItem *rowItem, const db::UserReleaseShortDetails &data)
{
    auto systemLocale = QLocale::system();

    rowItem->setText(0, data.title);
    if (data.rating.has_value())
    {
        rowItem->setText(1, (QString(data.rating.value(), u'★') + QString(5 - data.rating.value(), u'☆')));
    }
    rowItem->setText(2, data.platformId == "gog" ? "GOG.com" : "Other");
    rowItem->setText(3, data.tags);
    rowItem->setText(4, data.releaseDate.isNull() ? "-" : systemLocale.toString(data.releaseDate, QLocale::ShortFormat));
    rowItem->setText(5, "");
    if (!data.lastPlayedAt.isNull())
    {
        rowItem->setText(6, systemLocale.toString(data.lastPlayedAt, QLocale::ShortFormat));
    }
    if (data.totalPlaytime > 0)
    {
        QString gameplayStats;
        if (data.totalPlaytime > 60)
        {
            gameplayStats += systemLocale.toString(data.totalPlaytime / 60);
            gameplayStats += " h. ";
        }
        gameplayStats += systemLocale.toString(data.totalPlaytime % 60);
        gameplayStats += " min.";
        rowItem->setText(7, gameplayStats);
    }
    rowItem->setText(8, data.aggregatedRating.has_value()
                        ? systemLocale.toString(std::round(data.aggregatedRating.value()))
                        : "-");
    rowItem->setTextAlignment(8, Qt::AlignRight);
    rowItem->setText(9, data.genres);
    if (data.totalAchievementCount > 0)
    {
        rowItem->setText(10, QString("%1%2").arg(systemLocale.toString(std::round(100. * data.unlockedAchievementCount / data.totalAchievementCount)), systemLocale.percent()));
    }
}

void OwnedGamesPage::layoutData()
{
    int currentYear = QDate::currentDate().year();
    if (gridLayout)
    {
        QFont groupTitleFont;
        groupTitleFont.setBold(true);
        for (auto iconReply : std::as_const(listIconReplies))
        {
            iconReply->abort();
        }
        listIconReplies.clear();
        ui->resultsStack->setCurrentWidget(ui->resultsGridPage);
        ui->resultsTree->clear();
        for (const db::UserReleaseGroup &group : std::as_const(data))
        {
            QWidget *groupWidget = new QWidget(ui->resultsGridScrollAreaContents);
            QLayout *groupLayout = new FlowLayout(groupWidget, -1, 16, 12);
            groupWidget->setLayout(groupLayout);

            QString groupTitle = buildGroupTitle(group, request.grouping, currentYear);
            if (!groupTitle.isNull())
            {
                QLabel *groupTitleLabel = new QLabel(groupTitle, ui->resultsGridScrollAreaContents);
                groupTitleLabel->setFont(groupTitleFont);
                ui->resultsGridScrollAreaContentsLayout->addWidget(groupTitleLabel);
            }

            for (const db::UserReleaseShortDetails &item : std::as_const(group.items))
            {
                auto gridItem = new OwnedProductGridItem(item, apiClient, groupWidget);
                gridItem->setCursor(Qt::PointingHandCursor);
                gridItem->setImageSize(gridImageSizes[currentGridImageSize]);
                gridItem->setAdditionalDataDisplayed(OwnedProductGridItem::AdditionalInfo::PLATFORM);
                connect(this, &OwnedGamesPage::gridItemAdditionalDataVisibilityChanged,
                        gridItem, &OwnedProductGridItem::setAdditionalDataVisibility);
                connect(this, &OwnedGamesPage::gridItemAdditionalDataDisplayed,
                        gridItem, [gridItem](int kind)
                {
                    gridItem->setAdditionalDataDisplayed((OwnedProductGridItem::AdditionalInfo) kind);
                });
                connect(this, &OwnedGamesPage::gridItemImageSizeChanged,
                        gridItem, &OwnedProductGridItem::setImageSize);
                connect(this, &OwnedGamesPage::gridItemStatusVisibilityChanged,
                        gridItem, &OwnedProductGridItem::setStatusVisibility);
                connect(this, &OwnedGamesPage::gridItemRatingVisibilityChanged,
                        gridItem, &OwnedProductGridItem::setRatingVisibility);
                connect(this, &OwnedGamesPage::gridItemTitleVisibilityChanged,
                        gridItem, &OwnedProductGridItem::setTitleVisibility);
                connect(gridItem, &OwnedProductGridItem::clicked, this, [this, releaseId = item.id]()
                {
                    emit navigate({Page::RELEASE, releaseId});
                });
                groupLayout->addWidget(gridItem);
            }
            ui->resultsGridScrollAreaContentsLayout->addWidget(groupWidget);
        }
    }
    else
    {
        ui->resultsStack->setCurrentWidget(ui->resultsListPage);
        while (!ui->resultsGridScrollAreaContentsLayout->isEmpty())
        {
            auto item = ui->resultsGridScrollAreaContentsLayout->itemAt(0);
            ui->resultsGridScrollAreaContentsLayout->removeItem(item);
            item->widget()->deleteLater();
            delete item;
        }

        for (const db::UserReleaseGroup &group : std::as_const(data))
        {
            QString groupTitle = buildGroupTitle(group, request.grouping, currentYear);
            if (groupTitle.isNull())
            {
                for (const db::UserReleaseShortDetails &item : std::as_const(group.items))
                {
                    auto releaseItem = new QTreeWidgetItem(ui->resultsTree);
                    initializeUserReleaseRow(releaseItem, item);
                    if (!item.icon.isNull())
                    {
                        listIconReplies[item.id] = apiClient->getAnything(QString(item.icon).replace("{formatter}", "_glx_square_icon_v2").replace("{ext}", "webp"));
                        connect(listIconReplies[item.id], &QNetworkReply::finished, this, [this, id = item.id]()
                        {
                            auto networkReply = listIconReplies[id];
                            listIconReplies[id] = nullptr;
                            if (networkReply->error() == QNetworkReply::NoError)
                            {
                                QPixmap icon;
                                icon.loadFromData(networkReply->readAll());
                                emit listItemReceivedIcon(id, icon);
                            }
                            networkReply->deleteLater();
                        });
                        connect(this, &OwnedGamesPage::listItemReceivedIcon, listIconReplies[item.id], [releaseItem, id = item.id](const QString &releaseId, const QPixmap &icon)
                        {
                            if (id == releaseId)
                            {
                                releaseItem->setIcon(0, icon);
                            }
                        });
                    }
                }
            }
            else
            {
                auto groupItem = new QTreeWidgetItem(ui->resultsTree);
                groupItem->setExpanded(true);
                groupItem->setText(0, groupTitle);
                for (const db::UserReleaseShortDetails &item : std::as_const(group.items))
                {
                    auto releaseItem = new QTreeWidgetItem(groupItem);
                    initializeUserReleaseRow(releaseItem, item);
                    if (!item.icon.isNull())
                    {
                        listIconReplies[item.id] = apiClient->getAnything(QString(item.icon).replace("{formatter}", "_glx_square_icon_v2").replace("{ext}", "webp"));
                        connect(listIconReplies[item.id], &QNetworkReply::finished, this, [this, id = item.id]()
                        {
                            auto networkReply = listIconReplies[id];
                            listIconReplies[id] = nullptr;
                            if (networkReply->error() == QNetworkReply::NoError)
                            {
                                QPixmap icon;
                                icon.loadFromData(networkReply->readAll());
                                emit listItemReceivedIcon(id, icon);
                            }
                            networkReply->deleteLater();
                        });
                        connect(this, &OwnedGamesPage::listItemReceivedIcon, listIconReplies[item.id], [releaseItem, id = item.id](const QString &releaseId, const QPixmap &icon)
                        {
                            if (id == releaseId)
                            {
                                releaseItem->setIcon(0, icon);
                            }
                        });
                    }
                }
            }
        }
    }
}

void OwnedGamesPage::updateData()
{
    ui->contentsStack->setCurrentWidget(ui->loaderPage);
    ui->resultsGridScrollArea->verticalScrollBar()->setValue(0);
    while (!ui->resultsGridScrollAreaContentsLayout->isEmpty())
    {
        auto item = ui->resultsGridScrollAreaContentsLayout->itemAt(0);
        ui->resultsGridScrollAreaContentsLayout->removeItem(item);
        item->widget()->deleteLater();
        delete item;
    }
    for (auto iconReply : std::as_const(listIconReplies))
    {
        iconReply->abort();
    }
    listIconReplies.clear();
    ui->resultsTree->verticalScrollBar()->setValue(0);
    ui->resultsTree->clear();
    data.clear();

    data = db::getUserReleases(apiClient->currentUserId(), request);
    if (data.isEmpty())
    {
        ui->contentsStack->setCurrentWidget(ui->emptyPage);
    }
    else
    {
        layoutData();
        ui->contentsStack->setCurrentWidget(ui->resultsPage);
    }
}

void OwnedGamesPage::updateFilters()
{
    api::UserLibraryFilters filters = db::getUserReleasesFilters(apiClient->currentUserId());

    auto filterToolButton = static_cast<QToolButton *>(uiActions[1]);
    auto genresSubmenu = filterToolButton->menu()->actions()[0];
    genresSubmenu->setVisible(!filters.genres.empty());
    genresSubmenu->menu()->clear();
    for (const auto &genre : std::as_const(filters.genres))
    {
        auto genreItem = genresSubmenu->menu()->addAction(genre.name["*"].toString());
        genreItem->setCheckable(true);
        genreItem->setChecked(request.genres.contains(genre.id));
        connect(genreItem, &QAction::toggled, this, [this, genreId = genre.id](bool toggled)
        {
            if (toggled)
            {
                request.genres.insert(genreId);
            }
            else
            {
                request.genres.remove(genreId);
            }
            updateData();
        });
    }

    auto platformsSubmenu = filterToolButton->menu()->actions()[1];
    platformsSubmenu->setVisible(!filters.platforms.empty());
    platformsSubmenu->menu()->clear();
    for (const auto &platform : std::as_const(filters.platforms))
    {
        // TODO: map platform codes to names somehow
        QString platformName = platform == "gog" ? "GOG.com" : "Other";
        auto platformItem = platformsSubmenu->menu()->addAction(platformName);
        platformItem->setCheckable(true);
        platformItem->setChecked(request.platforms.contains(platform));
        connect(platformItem, &QAction::toggled, this, [this, platform](bool toggled)
        {
            if (toggled)
            {
                request.platforms.insert(platform);
            }
            else
            {
                request.platforms.remove(platform);
            }
            updateData();
        });
    }

    auto osSubmenu = filterToolButton->menu()->actions()[2];
    osSubmenu->setVisible(!filters.operatingSystems.empty());
    osSubmenu->menu()->clear();
    for (const auto &os : std::as_const(filters.operatingSystems))
    {
        auto osItem = osSubmenu->menu()->addAction(os.name);
        osItem->setCheckable(true);
        osItem->setChecked(request.supportedOs.contains(os.slug));
        connect(osItem, &QAction::toggled, this, [this, osSlug = os.slug](bool toggled)
        {
            if (toggled)
            {
                request.supportedOs.insert(osSlug);
            }
            else
            {
                request.supportedOs.remove(osSlug);
            }
            updateData();
        });
    }

    auto ratingsSubmenu = filterToolButton->menu()->actions()[3];
    ratingsSubmenu->setVisible(!filters.ratings.isEmpty());
    ratingsSubmenu->menu()->clear();
    for (const auto &rating : std::as_const(filters.ratings))
    {
        auto ratingItem = ratingsSubmenu->menu()->addAction((QString(rating, u'★') + QString(5 - rating, u'☆')));
        ratingItem->setCheckable(true);
        ratingItem->setChecked(request.ratings.contains(rating));
        connect(ratingItem, &QAction::toggled, this, [this, rating](bool toggled)
        {
            if (toggled)
            {
                request.ratings.insert(rating);
            }
            else
            {
                request.ratings.remove(rating);
            }
            updateData();
        });
    }
    if (filters.noRating)
    {
        auto ratingItem = ratingsSubmenu->menu()->addAction("No rating");
        ratingItem->setCheckable(true);
        ratingItem->setChecked(request.noRating);
        connect(ratingItem, &QAction::toggled, this, [this](bool toggled)
        {
            request.noRating = toggled;
            updateData();
        });
    }

    auto tagsSubmenu = filterToolButton->menu()->actions()[4];
    tagsSubmenu->setVisible(!filters.tags.isEmpty());
    tagsSubmenu->menu()->clear();
    for (const auto &tag : std::as_const(filters.tags))
    {
        auto tagItem = tagsSubmenu->menu()->addAction(tag);
        tagItem->setCheckable(true);
        tagItem->setChecked(request.tags.contains(tag));
        connect(tagItem, &QAction::toggled, this, [this, tag](bool toggled)
        {
            if (toggled)
            {
                request.tags.insert(tag);
            }
            else
            {
                request.tags.remove(tag);
            }
            updateData();
        });
    }
}

void OwnedGamesPage::initialize(const QVariant &data)
{
    updateFilters();
    updateData();
}

void OwnedGamesPage::switchUiAuthenticatedState(bool authenticated)
{

}


void OwnedGamesPage::on_retryButton_clicked()
{
    updateData();
}
