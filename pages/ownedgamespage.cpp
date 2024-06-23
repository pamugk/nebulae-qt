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
    gridImageSizes({ QSize(110, 155), QSize(120, 170), QSize(150, 210), QSize(185, 260), QSize(190, 265), QSize(260, 365) }),
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

    auto gridModeItem = viewSettingsMenu->addAction("Grid");
    gridModeItem->setCheckable(true);
    gridModeItem->setChecked(true);
    auto listModeItem = viewSettingsMenu->addAction("List");
    listModeItem->setCheckable(true);

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

    connect(gridModeItem, &QAction::toggled, this, [this, gridSettingsSubmenu, listModeItem](bool toggled)
    {
        if (toggled)
        {
        }
        listModeItem->setChecked(!toggled);
        gridSettingsSubmenu->menuAction()->setVisible(toggled);
    });

    auto settingsSubmenu = viewSettingsMenu->addMenu("Customize list view");
    auto listCustomizationItem = settingsSubmenu->addAction("Achievements %");
    listCustomizationItem->setCheckable(true);
    listCustomizationItem = settingsSubmenu->addAction("Critics rating");
    listCustomizationItem->setCheckable(true);
    listCustomizationItem = settingsSubmenu->addAction("Game icon");
    listCustomizationItem->setCheckable(true);
    listCustomizationItem = settingsSubmenu->addAction("Game time");
    listCustomizationItem->setCheckable(true);
    listCustomizationItem = settingsSubmenu->addAction("Genres");
    listCustomizationItem->setCheckable(true);
    listCustomizationItem = settingsSubmenu->addAction("Last played");
    listCustomizationItem->setCheckable(true);
    listCustomizationItem = settingsSubmenu->addAction("Platform icon");
    listCustomizationItem->setCheckable(true);
    listCustomizationItem = settingsSubmenu->addAction("Platform name");
    listCustomizationItem->setCheckable(true);
    listCustomizationItem = settingsSubmenu->addAction("Rating");
    listCustomizationItem->setCheckable(true);
    listCustomizationItem = settingsSubmenu->addAction("Release date");
    listCustomizationItem->setCheckable(true);
    listCustomizationItem = settingsSubmenu->addAction("Size on disk");
    listCustomizationItem->setCheckable(true);
    listCustomizationItem = settingsSubmenu->addAction("Status");
    listCustomizationItem->setCheckable(true);
    listCustomizationItem = settingsSubmenu->addAction("Tags");
    listCustomizationItem->setCheckable(true);
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
    groupingGroup->addAction(groupingAction);
    settingsSubmenu->addSeparator();
    groupingAction = settingsSubmenu->addAction("Critics rating");
    groupingAction->setCheckable(true);
    groupingAction->setChecked(request.grouping == api::UserReleaseGrouping::CRITICS_RATING_GROUP);
    groupingGroup->addAction(groupingAction);
    groupingAction = settingsSubmenu->addAction("Genre");
    groupingAction->setCheckable(true);
    groupingAction->setChecked(request.grouping == api::UserReleaseGrouping::GENRE_GROUP);
    groupingGroup->addAction(groupingAction);
    groupingAction = settingsSubmenu->addAction("Installed");
    groupingAction->setCheckable(true);
    groupingAction->setChecked(request.grouping == api::UserReleaseGrouping::INSTALLED_GROUP);
    groupingGroup->addAction(groupingAction);
    groupingAction = settingsSubmenu->addAction("Platform");
    groupingAction->setCheckable(true);
    groupingAction->setChecked(request.grouping == api::UserReleaseGrouping::PLATFORM_GROUP);
    groupingGroup->addAction(groupingAction);
    groupingAction = settingsSubmenu->addAction("Rating");
    groupingAction->setCheckable(true);
    groupingAction->setChecked(request.grouping == api::UserReleaseGrouping::RATING_GROUP);
    groupingGroup->addAction(groupingAction);
    groupingAction = settingsSubmenu->addAction("Release date");
    groupingAction->setCheckable(true);
    groupingAction->setChecked(request.grouping == api::UserReleaseGrouping::RELEASE_DATE_GROUP);
    groupingGroup->addAction(groupingAction);
    groupingAction = settingsSubmenu->addAction("Subscription");
    groupingAction->setCheckable(true);
    groupingAction->setChecked(request.grouping == api::UserReleaseGrouping::SUBSCRIPTION_GROUP);
    groupingGroup->addAction(groupingAction);
    groupingAction = settingsSubmenu->addAction("Tags");
    groupingAction->setCheckable(true);
    groupingAction->setChecked(request.grouping == api::UserReleaseGrouping::TAG_GROUP);
    groupingGroup->addAction(groupingAction);

    viewSettingsToolButton->setMenu(viewSettingsMenu);
    uiActions << viewSettingsToolButton;

    ui->contentsStack->setCurrentWidget(ui->loaderPage);
    ui->resultsStack->setCurrentWidget(ui->resultsGridPage);
    ui->resultsGridScrollAreaContents->setLayout(new FlowLayout(ui->resultsGridScrollAreaContents, -1, 16, 12));
}

OwnedGamesPage::~OwnedGamesPage()
{
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

void OwnedGamesPage::updateData()
{
    ui->contentsStack->setCurrentWidget(ui->loaderPage);
    ui->resultsGridScrollArea->verticalScrollBar()->setValue(0);
    while (!ui->resultsGridScrollAreaContents->layout()->isEmpty())
    {
        auto item = ui->resultsGridScrollAreaContents->layout()->itemAt(0);
        ui->resultsGridScrollAreaContents->layout()->removeItem(item);
        item->widget()->deleteLater();
        delete item;
    }

    QVector<db::UserReleaseShortDetails> releases = db::getUserReleases(apiClient->currentUserId(), request);
    if (releases.isEmpty())
    {
        ui->contentsStack->setCurrentWidget(ui->emptyPage);
    }
    else
    {
        for (const auto &item : std::as_const(releases))
        {
            auto gridItem = new OwnedProductGridItem(item, apiClient, ui->resultsGridPage);
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
            connect(gridItem, &OwnedProductGridItem::clicked, this, [this, productId = item.externalId]()
            {
                emit navigate({Page::OWNED_PRODUCT, productId});
            });
            ui->resultsGridScrollAreaContents->layout()->addWidget(gridItem);
        }
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
        QString platformName = platform == "gog" ? "GOG.com" : "Unknown platform";
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

