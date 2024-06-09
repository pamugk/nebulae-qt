#include "allgamespage.h"
#include "ui_allgamespage.h"

#include <algorithm>

#include <QCheckBox>
#include <QJsonDocument>
#include <QNetworkReply>
#include <QScrollBar>

#include "../api/models/catalog.h"
#include "../api/models/metatag.h"
#include "../api/utils/catalogserialization.h"
#include "../layouts/flowlayout.h"
#include "../widgets/clearfilterbutton.h"
#include "../widgets/collapsiblearea.h"
#include "../widgets/filtercheckbox.h"
#include "../widgets/storegridtile.h"
#include "../widgets/storelistitem.h"

AllGamesPage::AllGamesPage(QWidget *parent) :
    BasePage(parent),
    activatedFilterCount(0),
    filter({}),
    lastCatalogReply(nullptr),
    ui(new Ui::AllGamesPage)
{
    ui->setupUi(this);
    ui->filtersScrollAreaLayout->setAlignment(Qt::AlignTop);
    ui->resultsGridScrollAreaContents->setLayout(new FlowLayout(ui->resultsGridScrollAreaContents, -1, 24, 24));
    ui->resultsListLayout->setAlignment(Qt::AlignTop);
    gridLayout = true;

    currentSortOrder = 0;
    orders =
    {
        api::SortOrder{"trending", false},
        api::SortOrder{"bestselling", false},
        api::SortOrder{"price", true},
        api::SortOrder{"price", false},
        api::SortOrder{"discount", false},
        api::SortOrder{"title", true},
        api::SortOrder{"title", false},
        api::SortOrder{"releaseDate", false},
        api::SortOrder{"releaseDate", true},
        api::SortOrder{"reviewsRating", false}
    };

    filter.free = false;
    filter.discounted = false;
    filter.hideOwned = false;
    filter.onlyWishlisted = false;
    filter.productTypes = QStringList({"game","pack","dlc","extras"});

    ui->appliedFiltersHolder->setLayout(new FlowLayout(ui->appliedFiltersHolder, -1, 4, 4));

    page = 1;
    paginator = new Pagination(this);
    paginator->setVisible(false);
    connect(paginator, &Pagination::changedPage, this, [this](quint16 newPage)
    {
       page = newPage;
       fetchData();
    });
    ui->paginatorSlotLayout->addWidget(paginator, Qt::AlignHCenter);
}

AllGamesPage::~AllGamesPage()
{
    if (lastCatalogReply != nullptr)
    {
        lastCatalogReply->abort();
    }
    delete ui;
}

void AllGamesPage::setApiClient(api::GogApiClient *apiClient)
{
    this->apiClient = apiClient;
}

void AllGamesPage::fetchData()
{
    ui->contentsStack->setCurrentWidget(ui->loaderPage);
    data.products.clear();
    ui->resultsListScrollArea->verticalScrollBar()->setValue(0);
    while (!ui->resultsListLayout->isEmpty())
    {
        auto item = ui->resultsListLayout->itemAt(0);
        ui->resultsListLayout->removeItem(item);
        item->widget()->deleteLater();
        delete item;
    }
    ui->resultsGridScrollArea->verticalScrollBar()->setValue(0);
    while (!ui->resultsGridScrollAreaContents->layout()->isEmpty())
    {
        auto item = ui->resultsGridScrollAreaContents->layout()->itemAt(0);
        ui->resultsGridScrollAreaContents->layout()->removeItem(item);
        item->widget()->deleteLater();
        delete item;
    }

    if (lastCatalogReply != nullptr)
    {
        lastCatalogReply->abort();
    }

    auto systemLocale = QLocale::system();
    lastCatalogReply = apiClient->searchCatalog(orders[currentSortOrder], filter,
                                                QLocale::territoryToCode(systemLocale.territory()),
                                                systemLocale.name(QLocale::TagSeparator::Dash),
                                                systemLocale.currencySymbol(QLocale::CurrencyIsoCode), page);
    connect(lastCatalogReply, &QNetworkReply::finished, this, [this](){
        auto networkReply = lastCatalogReply;
        lastCatalogReply = nullptr;

        if (networkReply->error() == QNetworkReply::NoError)
        {
            auto resultJson = QJsonDocument::fromJson(QString(networkReply->readAll()).toUtf8()).object();
            parseSearchCatalogResponse(resultJson, data);

            ui->totalLabel->setText(QString("Showing %1 games").arg(QString::number(data.productCount)));
            ui->pagesLabel->setText(QString("%1 of %2").arg(QString::number(1), QString::number(data.pages)));

            if (data.products.isEmpty())
            {
                paginator->setVisible(false);
                ui->contentsStack->setCurrentWidget(ui->emptyPage);
            }
            else
            {
                paginator->setVisible(true);
                paginator->changePages(page, data.pages);
                layoutResults();
            }
        }
        else if (networkReply->error() != QNetworkReply::OperationCanceledError)
        {   ui->contentsStack->setCurrentWidget(ui->errorPage);
            qDebug() << networkReply->error() << networkReply->errorString() << QString(networkReply->readAll()).toUtf8();
        }

        networkReply->deleteLater();
    });
}

void AllGamesPage::layoutResults()
{
    ui->contentsStack->setCurrentWidget(ui->loaderPage);
    if (gridLayout)
    {
        for (const api::CatalogProduct &product : std::as_const(data.products))
        {
            auto storeItem = new StoreGridTile(product, apiClient, ui->resultsListPage);
            connect(apiClient, &api::GogApiClient::authenticated, storeItem,
                    &StoreGridTile::switchUiAuthenticatedState);
            storeItem->switchUiAuthenticatedState(apiClient->isAuthenticated());
            connect(storeItem, &StoreGridTile::navigateToProduct, this, [this](quint64 productId)
            {
                emit navigate({Page::CATALOG_PRODUCT, productId});
            });
            ui->resultsGridScrollAreaContents->layout()->addWidget(storeItem);
        }
        ui->resultsStackedWidget->setCurrentWidget(ui->resultsGridPage);
    }
    else
    {
        for (const api::CatalogProduct &product : std::as_const(data.products))
        {
            auto storeItem = new StoreListItem(product, apiClient, ui->resultsListPage);
            connect(apiClient, &api::GogApiClient::authenticated, storeItem,
                    &StoreListItem::switchUiAuthenticatedState);
            storeItem->switchUiAuthenticatedState(apiClient->isAuthenticated());
            connect(storeItem, &StoreListItem::navigateToProduct, this, [this](quint64 productId)
            {
                emit navigate({Page::CATALOG_PRODUCT, productId});
            });
            ui->resultsListLayout->addWidget(storeItem);
        }
        ui->resultsStackedWidget->setCurrentWidget(ui->resultsListPage);
    }
    ui->contentsStack->setCurrentWidget(ui->resultsPage);
}

void AllGamesPage::initialize(const QVariant &data)
{
    QMap<QString, QVariant> initialFilters = data.toMap();
    if (initialFilters.contains("discounted"))
    {
        filter.discounted = initialFilters["discounted"].toBool();
    }
    if (initialFilters.contains("developer"))
    {
        filter.developers.append(initialFilters["developer"].toString());
        auto clearFilterButton = new ClearFilterButton("Selected developer", QString(), ui->appliedFiltersHolder);
        ui->appliedFiltersHolder->layout()->addWidget(clearFilterButton);
        connect(clearFilterButton, &ClearFilterButton::clicked, this, [this, clearFilterButton]()
        {
            filter.developers.clear();
            ui->appliedFiltersHolder->layout()->removeWidget(clearFilterButton);
            clearFilterButton->setVisible(false);
            page = 1;
            fetchData();
        });
    }
    if (initialFilters.contains("publisher"))
    {
        filter.publishers.append(initialFilters["publisher"].toString());
        auto clearFilterButton = new ClearFilterButton("Selected publisher", QString(), ui->appliedFiltersHolder);
        ui->appliedFiltersHolder->layout()->addWidget(clearFilterButton);
        connect(clearFilterButton, &ClearFilterButton::clicked, this, [this, clearFilterButton]()
        {
            filter.publishers.clear();
            ui->appliedFiltersHolder->layout()->removeWidget(clearFilterButton);
            clearFilterButton->setVisible(false);
            page = 1;
            fetchData();
        });
    }
    if (initialFilters.contains("genre"))
    {
        filter.genres.append(initialFilters["genre"].toString());
    }
    if (initialFilters.contains("tag"))
    {
        filter.tags.append(initialFilters["tag"].toString());
    }
    if (initialFilters.contains("feature"))
    {
        filter.features.append(initialFilters["feature"].toString());
    }
    if (initialFilters.contains("releaseStatus"))
    {
        filter.releaseStatuses.append(initialFilters["releaseStatus"].toString());
    }
    auto systemLocale = QLocale::system();
    lastCatalogReply = apiClient->searchCatalog(orders[currentSortOrder], filter,
                                                QLocale::territoryToCode(systemLocale.territory()),
                                                systemLocale.name(QLocale::TagSeparator::Dash),
                                                systemLocale.currencySymbol(QLocale::CurrencyIsoCode), page);
    ui->filtersScrollArea->setVisible(false);
    ui->contentsStack->setCurrentWidget(ui->loaderPage);
    connect(lastCatalogReply, &QNetworkReply::finished, this, [this](){
        auto networkReply = lastCatalogReply;
        lastCatalogReply = nullptr;

        if (networkReply->error() == QNetworkReply::NoError)
        {
            auto resultJson = QJsonDocument::fromJson(QString(networkReply->readAll()).toUtf8()).object();
            parseSearchCatalogResponse(resultJson, this->data);

            int maxWidth = 0;
            CollapsibleArea *area;
            QLayout *layout;
            QCheckBox *checkbox;
            ClearFilterButton *clearFilterButton;

            clearFilterButton = new ClearFilterButton("Discounted", QString(), ui->appliedFiltersHolder);
            checkbox = new QCheckBox("Show only discounted", ui->filtersScrollAreaContents);
            maxWidth = std::max(maxWidth, checkbox->width());
            checkbox->setChecked(filter.discounted);
            if (checkbox->isChecked())
            {
                activatedFilterCount++;
                ui->appliedFiltersHolder->layout()->addWidget(clearFilterButton);
            }
            else
            {
                clearFilterButton->setVisible(false);
            }
            connect(clearFilterButton, &ClearFilterButton::clicked, checkbox, [this, checkbox]()
            {
               checkbox->setChecked(false);
            });
            connect(checkbox, &QCheckBox::toggled, this, [this, clearFilterButton](bool value)
            {
                if (value)
                {
                    activatedFilterCount++;
                    ui->appliedFiltersHolder->layout()->addWidget(clearFilterButton);
                    clearFilterButton->setVisible(true);
                }
                else
                {
                    activatedFilterCount--;
                    ui->appliedFiltersHolder->layout()->removeWidget(clearFilterButton);
                    clearFilterButton->setVisible(false);
                }
                filter.discounted = value;
                page = 1;
                fetchData();
            });
            ui->filtersScrollAreaLayout->addWidget(checkbox);

            clearFilterButton = new ClearFilterButton("Showing without DLCs and extras", QString(), ui->appliedFiltersHolder);
            checkbox = new QCheckBox("Hide DLCs and extras", ui->filtersScrollAreaContents);
            checkbox->setChecked(!filter.productTypes.contains("dlc"));
            if (checkbox->isChecked())
            {
                activatedFilterCount++;
                ui->appliedFiltersHolder->layout()->addWidget(clearFilterButton);
            }
            else
            {
                clearFilterButton->setVisible(false);
            }
            maxWidth = std::max(maxWidth, checkbox->width());
            connect(clearFilterButton, &ClearFilterButton::clicked, checkbox, [this, checkbox]()
            {
               checkbox->setChecked(false);
            });
            connect(checkbox, &QCheckBox::toggled, this, [this, clearFilterButton](bool value)
            {
                if (value)
                {
                    activatedFilterCount++;
                    filter.productTypes = QStringList({"game","pack"});
                    ui->appliedFiltersHolder->layout()->addWidget(clearFilterButton);
                    clearFilterButton->setVisible(true);
                }
                else
                {
                    activatedFilterCount--;
                    filter.productTypes = QStringList({"game","pack","dlc","extras"});
                    ui->appliedFiltersHolder->layout()->removeWidget(clearFilterButton);
                    clearFilterButton->setVisible(false);
                }
                page = 1;
                fetchData();
            });
            ui->filtersScrollAreaLayout->addWidget(checkbox);

            clearFilterButton = new ClearFilterButton("Excluding owned products", QString(), ui->appliedFiltersHolder);
            clearFilterButton->setVisible(false);
            checkbox = new QCheckBox("Hide all owned products", ui->filtersScrollAreaContents);
            checkbox->setChecked(filter.hideOwned);
            checkbox->setVisible(false);
            connect(clearFilterButton, &ClearFilterButton::clicked, checkbox, [this, checkbox]()
            {
               checkbox->setChecked(false);
            });
            connect(checkbox, &QCheckBox::toggled, this, [this, clearFilterButton](bool value)
            {
                if (value)
                {
                    activatedFilterCount++;
                    ui->appliedFiltersHolder->layout()->addWidget(clearFilterButton);
                    clearFilterButton->setVisible(true);
                }
                else
                {
                    activatedFilterCount--;
                    ui->appliedFiltersHolder->layout()->removeWidget(clearFilterButton);
                    clearFilterButton->setVisible(false);
                }
                filter.hideOwned = value;
                page = 1;
                fetchData();
            });
            ui->filtersScrollAreaLayout->addWidget(checkbox);

            clearFilterButton = new ClearFilterButton("Showing only wishlisted games", QString(), ui->appliedFiltersHolder);
            clearFilterButton->setVisible(false);
            checkbox = new QCheckBox("Show only games on my wishlist", ui->filtersScrollAreaContents);
            checkbox->setChecked(filter.onlyWishlisted);
            checkbox->setVisible(false);
            connect(clearFilterButton, &ClearFilterButton::clicked, checkbox, [this, checkbox]()
            {
               checkbox->setChecked(false);
            });
            connect(checkbox, &QCheckBox::toggled, this, [this, clearFilterButton](bool value)
            {
                if (value)
                {
                    activatedFilterCount++;
                    ui->appliedFiltersHolder->layout()->addWidget(clearFilterButton);
                    clearFilterButton->setVisible(true);
                }
                else
                {
                    activatedFilterCount--;
                    ui->appliedFiltersHolder->layout()->removeWidget(clearFilterButton);
                    clearFilterButton->setVisible(false);
                }
                filter.onlyWishlisted = value;
                page = 1;
                fetchData();
            });
            ui->filtersScrollAreaLayout->addWidget(checkbox);

            area = new CollapsibleArea("Price range", ui->filtersScrollAreaContents);
            layout = new QVBoxLayout();
            clearFilterButton = new ClearFilterButton("Price Range", "0 - 0", ui->appliedFiltersHolder);
            clearFilterButton->setVisible(false);
            checkbox = new QCheckBox("Show only free games", area);
            connect(clearFilterButton, &ClearFilterButton::clicked, checkbox, [this, checkbox]()
            {
               checkbox->setChecked(false);
            });
            connect(checkbox, &QCheckBox::toggled, this, [this, clearFilterButton](bool value)
            {
                if (value)
                {
                    activatedFilterCount++;
                    ui->appliedFiltersHolder->layout()->addWidget(clearFilterButton);
                    clearFilterButton->setVisible(true);
                }
                else
                {
                    activatedFilterCount--;
                    ui->appliedFiltersHolder->layout()->removeWidget(clearFilterButton);
                    clearFilterButton->setVisible(false);
                }
                filter.free = value;
                page = 1;
                fetchData();
            });
            layout->addWidget(checkbox);
            area->setContentLayout(layout);
            maxWidth = std::max(maxWidth, layout->sizeHint().width());
            ui->filtersScrollAreaLayout->addWidget(area);

            area = new CollapsibleArea("Release Status", ui->filtersScrollAreaContents);
            area->setChangedFilters(filter.releaseStatuses.count() + filter.excludeReleaseStatuses.count());
            layout = new QVBoxLayout();
            layout->setAlignment(Qt::AlignTop);
            for (const api::MetaTag &item : std::as_const(this->data.filters.releaseStatuses))
            {
                auto clearFilterButton = new ClearFilterButton("Release status", item.name, ui->appliedFiltersHolder);
                auto clearHideFilterButton = new ClearFilterButton("Hide release status", item.name, ui->appliedFiltersHolder);
                auto filterCheckbox = new FilterCheckbox(item.name, area);
                bool shouldInclude = filter.releaseStatuses.contains(item.slug);
                filterCheckbox->setInclude(shouldInclude);
                if (shouldInclude)
                {
                    activatedFilterCount++;
                    ui->appliedFiltersHolder->layout()->addWidget(clearFilterButton);
                }
                else
                {
                    clearFilterButton->setVisible(false);
                }
                bool shouldExclude = filter.excludeReleaseStatuses.contains(item.slug);
                filterCheckbox->setExclude(shouldExclude);
                if (shouldExclude)
                {
                    activatedFilterCount++;
                    ui->appliedFiltersHolder->layout()->addWidget(clearHideFilterButton);
                }
                else
                {
                    clearHideFilterButton->setVisible(false);
                }
                connect(clearFilterButton, &ClearFilterButton::clicked, filterCheckbox, [this, filterCheckbox]()
                {
                   filterCheckbox->setInclude(false);
                });
                connect(filterCheckbox, &FilterCheckbox::include, this, [this, item, area, clearFilterButton](bool shouldInclude)
                {
                    if (shouldInclude)
                    {
                        activatedFilterCount++;
                        filter.releaseStatuses.append(item.slug);
                        ui->appliedFiltersHolder->layout()->addWidget(clearFilterButton);
                        clearFilterButton->setVisible(true);
                    }
                    else
                    {
                        activatedFilterCount--;
                        filter.releaseStatuses.removeOne(item.slug);
                        ui->appliedFiltersHolder->layout()->removeWidget(clearFilterButton);
                        clearFilterButton->setVisible(false);
                    }
                    page = 1;
                    area->setChangedFilters(filter.releaseStatuses.count() + filter.excludeReleaseStatuses.count());
                    fetchData();
                });
                connect(clearHideFilterButton, &ClearFilterButton::clicked, filterCheckbox, [this, filterCheckbox]()
                {
                   filterCheckbox->setExclude(false);
                });
                connect(filterCheckbox, &FilterCheckbox::exclude, this, [this, item, area, clearHideFilterButton](bool shouldExclude)
                {
                    if (shouldExclude)
                    {
                        activatedFilterCount++;
                        filter.excludeReleaseStatuses.append(item.slug);
                        ui->appliedFiltersHolder->layout()->addWidget(clearHideFilterButton);
                        clearHideFilterButton->setVisible(true);
                    }
                    else
                    {
                        activatedFilterCount--;
                        filter.excludeReleaseStatuses.removeOne(item.slug);
                        ui->appliedFiltersHolder->layout()->removeWidget(clearHideFilterButton);
                        clearHideFilterButton->setVisible(false);
                    }
                    page = 1;
                    area->setChangedFilters(filter.releaseStatuses.count() + filter.excludeReleaseStatuses.count());
                    fetchData();
                });
                layout->addWidget(filterCheckbox);
            }
            area->setContentLayout(layout);
            maxWidth = std::max(maxWidth, layout->sizeHint().width());
            ui->filtersScrollAreaLayout->addWidget(area);

            area = new CollapsibleArea("Genres", ui->filtersScrollAreaContents);
            area->setChangedFilters(filter.genres.count() + filter.excludeGenres.count());
            layout = new QVBoxLayout();
            layout->setAlignment(Qt::AlignTop);
            for (const api::MetaTag &item : std::as_const(this->data.filters.genres))
            {
                auto clearFilterButton = new ClearFilterButton("Genre", item.name, ui->appliedFiltersHolder);
                auto clearHideFilterButton = new ClearFilterButton("Hide genre", item.name, ui->appliedFiltersHolder);
                auto filterCheckbox = new FilterCheckbox(item.name, area);
                bool shouldInclude = filter.genres.contains(item.slug);
                filterCheckbox->setInclude(shouldInclude);
                if (shouldInclude)
                {
                    activatedFilterCount++;
                    ui->appliedFiltersHolder->layout()->addWidget(clearFilterButton);
                }
                else
                {
                    clearFilterButton->setVisible(false);
                }
                bool shouldExclude = filter.excludeGenres.contains(item.slug);
                filterCheckbox->setExclude(shouldExclude);
                if (shouldExclude)
                {
                    activatedFilterCount++;
                    ui->appliedFiltersHolder->layout()->addWidget(clearHideFilterButton);
                }
                else
                {
                    clearHideFilterButton->setVisible(false);
                }
                connect(clearFilterButton, &ClearFilterButton::clicked, filterCheckbox, [this, filterCheckbox]()
                {
                   filterCheckbox->setInclude(false);
                });
                connect(filterCheckbox, &FilterCheckbox::include, this, [this, item, area, clearFilterButton](bool shouldInclude)
                {
                    if (shouldInclude)
                    {
                        activatedFilterCount++;
                        filter.genres.append(item.slug);
                        ui->appliedFiltersHolder->layout()->addWidget(clearFilterButton);
                        clearFilterButton->setVisible(true);
                    }
                    else
                    {
                        activatedFilterCount--;
                        filter.genres.removeOne(item.slug);
                        ui->appliedFiltersHolder->layout()->removeWidget(clearFilterButton);
                        clearFilterButton->setVisible(false);
                    }
                    page = 1;
                    area->setChangedFilters(filter.genres.count() + filter.excludeGenres.count());
                    fetchData();
                });
                connect(clearHideFilterButton, &ClearFilterButton::clicked, filterCheckbox, [this, filterCheckbox]()
                {
                   filterCheckbox->setExclude(false);
                });
                connect(filterCheckbox, &FilterCheckbox::exclude, this, [this, item, area, clearHideFilterButton](bool shouldExclude)
                {
                    if (shouldExclude)
                    {
                        activatedFilterCount++;
                        filter.excludeGenres.append(item.slug);
                        ui->appliedFiltersHolder->layout()->addWidget(clearHideFilterButton);
                        clearHideFilterButton->setVisible(true);
                    }
                    else
                    {
                        activatedFilterCount--;
                        filter.excludeGenres.removeOne(item.slug);
                        ui->appliedFiltersHolder->layout()->removeWidget(clearHideFilterButton);
                        clearHideFilterButton->setVisible(false);
                    }
                    page = 1;
                    area->setChangedFilters(filter.genres.count() + filter.excludeGenres.count());
                    fetchData();
                });
                layout->addWidget(filterCheckbox);
            }
            area->setContentLayout(layout);
            maxWidth = std::max(maxWidth, layout->sizeHint().width());
            ui->filtersScrollAreaLayout->addWidget(area);

            area = new CollapsibleArea("Tags", ui->filtersScrollAreaContents);
            area->setChangedFilters(filter.tags.count() + filter.excludeTags.count());
            layout = new QVBoxLayout();
            layout->setAlignment(Qt::AlignTop);
            for (const api::MetaTag &item : std::as_const(this->data.filters.tags))
            {
                auto clearFilterButton = new ClearFilterButton("Tag", item.name, ui->appliedFiltersHolder);
                auto clearHideFilterButton = new ClearFilterButton("Hide tag", item.name, ui->appliedFiltersHolder);
                auto filterCheckbox = new FilterCheckbox(item.name, area);
                bool shouldInclude = filter.tags.contains(item.slug);
                filterCheckbox->setInclude(shouldInclude);
                if (shouldInclude)
                {
                    activatedFilterCount++;
                    ui->appliedFiltersHolder->layout()->addWidget(clearFilterButton);
                }
                else
                {
                    clearFilterButton->setVisible(false);
                }
                bool shouldExclude = filter.excludeTags.contains(item.slug);
                filterCheckbox->setExclude(shouldExclude);
                if (shouldExclude)
                {
                    activatedFilterCount++;
                    ui->appliedFiltersHolder->layout()->addWidget(clearHideFilterButton);
                }
                else
                {
                    clearHideFilterButton->setVisible(false);
                }
                connect(clearFilterButton, &ClearFilterButton::clicked, filterCheckbox, [this, filterCheckbox]()
                {
                   filterCheckbox->setInclude(false);
                });
                connect(filterCheckbox, &FilterCheckbox::include, this, [this, item, area, clearFilterButton](bool shouldInclude)
                {
                    if (shouldInclude)
                    {
                        activatedFilterCount++;
                        filter.tags.append(item.slug);
                        ui->appliedFiltersHolder->layout()->addWidget(clearFilterButton);
                        clearFilterButton->setVisible(true);
                    }
                    else
                    {
                        activatedFilterCount--;
                        filter.tags.removeOne(item.slug);
                        ui->appliedFiltersHolder->layout()->removeWidget(clearFilterButton);
                        clearFilterButton->setVisible(false);
                    }
                    page = 1;
                    area->setChangedFilters(filter.tags.count() + filter.excludeTags.count());
                    fetchData();
                });
                connect(clearHideFilterButton, &ClearFilterButton::clicked, filterCheckbox, [this, filterCheckbox]()
                {
                   filterCheckbox->setExclude(false);
                });
                connect(filterCheckbox, &FilterCheckbox::exclude, this, [this, item, area, clearHideFilterButton](bool shouldExclude)
                {
                    if (shouldExclude)
                    {
                        activatedFilterCount++;
                        filter.excludeTags.append(item.slug);
                        ui->appliedFiltersHolder->layout()->addWidget(clearHideFilterButton);
                        clearHideFilterButton->setVisible(true);
                    }
                    else
                    {
                        activatedFilterCount--;
                        filter.excludeTags.removeOne(item.slug);
                        ui->appliedFiltersHolder->layout()->removeWidget(clearHideFilterButton);
                        clearHideFilterButton->setVisible(false);
                    }
                    page = 1;
                    area->setChangedFilters(filter.tags.count() + filter.excludeTags.count());
                    fetchData();
                });
                layout->addWidget(filterCheckbox);
            }
            layout->setAlignment(Qt::AlignTop);
            area->setContentLayout(layout);
            maxWidth = std::max(maxWidth, layout->sizeHint().width());
            ui->filtersScrollAreaLayout->addWidget(area);

            area = new CollapsibleArea("Operating Systems", ui->filtersScrollAreaContents);
            area->setChangedFilters(filter.systems.count());
            layout = new QVBoxLayout();
            layout->setAlignment(Qt::AlignTop);
            for (const api::MetaTag &item : std::as_const(this->data.filters.systems))
            {
                clearFilterButton = new ClearFilterButton("Operating System", item.name, ui->appliedFiltersHolder);
                auto checkbox = new QCheckBox(item.name, area);
                checkbox->setChecked(filter.systems.contains(item.slug));
                if (checkbox->isChecked())
                {
                    activatedFilterCount++;
                    ui->appliedFiltersHolder->layout()->addWidget(clearFilterButton);
                }
                else
                {
                    clearFilterButton->setVisible(false);
                }
                connect(clearFilterButton, &ClearFilterButton::clicked, checkbox, [this, checkbox]()
                {
                   checkbox->setChecked(false);
                });
                connect(checkbox, &QCheckBox::toggled, this, [this, item, area, clearFilterButton](bool shouldInclude)
                {
                    if (shouldInclude)
                    {
                        activatedFilterCount++;
                        ui->appliedFiltersHolder->layout()->addWidget(clearFilterButton);
                        clearFilterButton->setVisible(true);
                        filter.systems.append(item.slug);
                    }
                    else
                    {
                        activatedFilterCount--;
                        filter.systems.removeOne(item.slug);
                        ui->appliedFiltersHolder->layout()->removeWidget(clearFilterButton);
                        clearFilterButton->setVisible(false);
                    }
                    page = 1;
                    area->setChangedFilters(filter.systems.count());
                    fetchData();
                });
                layout->addWidget(checkbox);
            }
            area->setContentLayout(layout);
            maxWidth = std::max(maxWidth, layout->sizeHint().width());
            ui->filtersScrollAreaLayout->addWidget(area);

            area = new CollapsibleArea("Features", ui->filtersScrollAreaContents);
            area->setChangedFilters(filter.features.count() + filter.excludeFeatures.count());
            layout = new QVBoxLayout();
            layout->setAlignment(Qt::AlignTop);
            for (const api::MetaTag &item : std::as_const(this->data.filters.features))
            {
                auto clearFilterButton = new ClearFilterButton("Feature", item.name, ui->appliedFiltersHolder);
                auto clearHideFilterButton = new ClearFilterButton("Hide feature", item.name, ui->appliedFiltersHolder);
                auto filterCheckbox = new FilterCheckbox(item.name, area);
                bool shouldInclude = filter.features.contains(item.slug);
                filterCheckbox->setInclude(shouldInclude);
                if (shouldInclude)
                {
                    activatedFilterCount++;
                    ui->appliedFiltersHolder->layout()->addWidget(clearFilterButton);
                }
                else
                {
                    clearFilterButton->setVisible(false);
                }
                bool shouldExclude = filter.excludeFeatures.contains(item.slug);
                filterCheckbox->setExclude(shouldExclude);
                if (shouldExclude)
                {
                    activatedFilterCount++;
                    ui->appliedFiltersHolder->layout()->addWidget(clearHideFilterButton);
                }
                else
                {
                    clearHideFilterButton->setVisible(false);
                }
                connect(clearFilterButton, &ClearFilterButton::clicked, filterCheckbox, [this, filterCheckbox]()
                {
                   filterCheckbox->setInclude(false);
                });
                connect(filterCheckbox, &FilterCheckbox::include, this, [this, item, area, clearFilterButton](bool shouldInclude)
                {
                    if (shouldInclude)
                    {
                        activatedFilterCount++;
                        filter.features.append(item.slug);
                        ui->appliedFiltersHolder->layout()->addWidget(clearFilterButton);
                        clearFilterButton->setVisible(true);
                    }
                    else
                    {
                        activatedFilterCount--;
                        filter.features.removeOne(item.slug);
                        ui->appliedFiltersHolder->layout()->removeWidget(clearFilterButton);
                        clearFilterButton->setVisible(false);
                    }
                    page = 1;
                    area->setChangedFilters(filter.features.count() + filter.excludeFeatures.count());
                    fetchData();
                });
                connect(clearHideFilterButton, &ClearFilterButton::clicked, filterCheckbox, [this, filterCheckbox]()
                {
                   filterCheckbox->setExclude(false);
                });
                connect(filterCheckbox, &FilterCheckbox::exclude, this, [this, item, area, clearHideFilterButton](bool shouldExclude)
                {
                    if (shouldExclude)
                    {
                        activatedFilterCount++;
                        filter.excludeFeatures.append(item.slug);
                        ui->appliedFiltersHolder->layout()->addWidget(clearHideFilterButton);
                        clearHideFilterButton->setVisible(true);
                    }
                    else
                    {
                        activatedFilterCount--;
                        filter.excludeFeatures.removeOne(item.slug);
                        ui->appliedFiltersHolder->layout()->removeWidget(clearHideFilterButton);
                        clearHideFilterButton->setVisible(false);
                    }
                    page = 1;
                    area->setChangedFilters(filter.features.count() + filter.excludeFeatures.count());
                    fetchData();
                });
                layout->addWidget(filterCheckbox);
            }
            area->setContentLayout(layout);
            maxWidth = std::max(maxWidth, layout->sizeHint().width());
            ui->filtersScrollAreaLayout->addWidget(area);

            area = new CollapsibleArea("Release Date", ui->filtersScrollAreaContents);
            ui->filtersScrollAreaLayout->addWidget(area);

            area = new CollapsibleArea("Languages", ui->filtersScrollAreaContents);
            area->setChangedFilters(filter.languages.count());
            layout = new QVBoxLayout();
            layout->setAlignment(Qt::AlignTop);
            for (const api::MetaTag &item : std::as_const(this->data.filters.languages))
            {
                clearFilterButton = new ClearFilterButton("Language", item.name, ui->appliedFiltersHolder);
                auto checkbox = new QCheckBox(item.name, area);
                checkbox->setChecked(filter.languages.contains(item.slug));
                if (checkbox->isChecked())
                {
                    activatedFilterCount++;
                    ui->appliedFiltersHolder->layout()->addWidget(clearFilterButton);
                }
                else
                {
                    clearFilterButton->setVisible(false);
                }
                connect(clearFilterButton, &ClearFilterButton::clicked, checkbox, [this, checkbox]()
                {
                   checkbox->setChecked(false);
                });
                connect(checkbox, &QCheckBox::toggled, this, [this, item, area, clearFilterButton](bool shouldInclude)
                {
                    if (shouldInclude)
                    {
                        activatedFilterCount++;
                        filter.languages.append(item.slug);
                        ui->appliedFiltersHolder->layout()->addWidget(clearFilterButton);
                        clearFilterButton->setVisible(true);
                    }
                    else
                    {
                        activatedFilterCount--;
                        filter.languages.removeOne(item.slug);
                        ui->appliedFiltersHolder->layout()->removeWidget(clearFilterButton);
                        clearFilterButton->setVisible(false);
                    }
                    page = 1;
                    area->setChangedFilters(filter.languages.count());
                    fetchData();
                });
                layout->addWidget(checkbox);
            }
            area->setContentLayout(layout);
            maxWidth = std::max(maxWidth, layout->sizeHint().width());
            ui->filtersScrollAreaLayout->addWidget(area);

            ui->filtersScrollArea->setFixedWidth(maxWidth + 20);
            ui->filtersScrollArea->setVisible(true);

            ui->totalLabel->setText(QString("Showing %1 games").arg(QString::number(this->data.productCount)));
            ui->pagesLabel->setText(QString("%1 of %2").arg(QString::number(page), QString::number(this->data.pages)));

            if (this->data.products.isEmpty())
            {
                paginator->setVisible(false);
                ui->contentsStack->setCurrentWidget(ui->emptyPage);
            }
            else
            {
                paginator->setVisible(true);
                paginator->changePages(page, this->data.pages);
                layoutResults();
            }
        }
        else if (networkReply->error() != QNetworkReply::OperationCanceledError)
        {   ui->contentsStack->setCurrentWidget(ui->errorPage);
            qDebug() << networkReply->error() << networkReply->errorString() << QString(networkReply->readAll()).toUtf8();
        }

        networkReply->deleteLater();
    });
}

void AllGamesPage::switchUiAuthenticatedState(bool authenticated)
{

}

void AllGamesPage::on_lineEdit_textChanged(const QString &arg1)
{
    page = 1;
    filter.query = arg1.trimmed();
    fetchData();
}


void AllGamesPage::on_sortOrderComboBox_currentIndexChanged(int index)
{
    if (index < 0 || index > 9)
    {
        index = 0;
    }
    page = 1;
    currentSortOrder = index;
    fetchData();
}


void AllGamesPage::on_gridModeButton_clicked()
{
    if (!gridLayout)
    {
        gridLayout = true;
        while (!ui->resultsListLayout->isEmpty())
        {
            auto item = ui->resultsListLayout->itemAt(0);
            ui->resultsListLayout->removeItem(item);
            item->widget()->deleteLater();
            delete item;
        }
        layoutResults();
    }
}


void AllGamesPage::on_listModeButton_clicked()
{
    if (gridLayout)
    {
        gridLayout = false;
        while (!ui->resultsGridScrollAreaContents->layout()->isEmpty())
        {
            auto item = ui->resultsGridScrollAreaContents->layout()->itemAt(0);
            ui->resultsGridScrollAreaContents->layout()->removeItem(item);
            item->widget()->deleteLater();
            delete item;
        }
        layoutResults();
    }
}

