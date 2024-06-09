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
    applyFilters(true),
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
    }
    if (initialFilters.contains("publisher"))
    {
        filter.publishers.append(initialFilters["publisher"].toString());
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

            auto clearAllFiltersButton = new ClearFilterButton("Clear all filters", QString(), ui->appliedFiltersHolder);
            clearAllFiltersButton->setVisible(false);
            ui->appliedFiltersHolder->layout()->addWidget(clearAllFiltersButton);
            connect(clearAllFiltersButton, &ClearFilterButton::clicked, this, [this, clearAllFiltersButton]()
            {
                applyFilters = false;
                while (ui->appliedFiltersHolder->layout()->count() > 1)
                {
                    emit static_cast<ClearFilterButton*>(ui->appliedFiltersHolder->layout()->itemAt(1)->widget())->clicked();
                }
                applyFilters = true;

                page = 1;
                fetchData();
            });

            int maxWidth = 0;
            CollapsibleArea *area;
            QLayout *layout;
            QCheckBox *checkbox;
            ClearFilterButton *clearFilterButton;

            if (!filter.developers.isEmpty())
            {
                activatedFilterCount++;
                clearAllFiltersButton->setVisible(activatedFilterCount > 1);
                auto clearFilterButton = new ClearFilterButton("Selected developer", QString(), ui->appliedFiltersHolder);
                ui->appliedFiltersHolder->layout()->addWidget(clearFilterButton);
                connect(clearFilterButton, &ClearFilterButton::clicked, this, [this, clearFilterButton, clearAllFiltersButton]()
                {
                    activatedFilterCount--;
                    clearAllFiltersButton->setVisible(activatedFilterCount > 1);
                    filter.developers.clear();
                    ui->appliedFiltersHolder->layout()->removeWidget(clearFilterButton);
                    clearFilterButton->setVisible(false);

                    if (applyFilters)
                    {
                        page = 1;
                        fetchData();
                    }
                });
            }
            if (!filter.publishers.isEmpty())
            {
                activatedFilterCount++;
                clearAllFiltersButton->setVisible(activatedFilterCount > 1);
                auto clearFilterButton = new ClearFilterButton("Selected publisher", QString(), ui->appliedFiltersHolder);
                ui->appliedFiltersHolder->layout()->addWidget(clearFilterButton);
                connect(clearFilterButton, &ClearFilterButton::clicked, this, [this, clearFilterButton, clearAllFiltersButton]()
                {
                    activatedFilterCount--;
                    clearAllFiltersButton->setVisible(activatedFilterCount > 1);
                    filter.publishers.clear();
                    ui->appliedFiltersHolder->layout()->removeWidget(clearFilterButton);
                    clearFilterButton->setVisible(false);

                    if (applyFilters)
                    {
                        page = 1;
                        fetchData();
                    }
                });
            }

            clearFilterButton = new ClearFilterButton("Discounted", QString(), ui->appliedFiltersHolder);
            checkbox = new QCheckBox("Show only discounted", ui->filtersScrollAreaContents);
            maxWidth = std::max(maxWidth, checkbox->width());
            checkbox->setChecked(filter.discounted);
            if (checkbox->isChecked())
            {
                activatedFilterCount++;
                clearAllFiltersButton->setVisible(activatedFilterCount > 1);
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
            connect(checkbox, &QCheckBox::toggled,
                    this, [this, clearFilterButton, clearAllFiltersButton](bool value)
            {
                if (value)
                {
                    activatedFilterCount++;
                    clearAllFiltersButton->setVisible(activatedFilterCount > 1);
                    ui->appliedFiltersHolder->layout()->addWidget(clearFilterButton);
                    clearFilterButton->setVisible(true);
                }
                else
                {
                    activatedFilterCount--;
                    clearAllFiltersButton->setVisible(activatedFilterCount > 1);
                    ui->appliedFiltersHolder->layout()->removeWidget(clearFilterButton);
                    clearFilterButton->setVisible(false);
                }
                filter.discounted = value;

                if (applyFilters)
                {
                    page = 1;
                    fetchData();
                }
            });
            ui->filtersScrollAreaLayout->addWidget(checkbox);

            clearFilterButton = new ClearFilterButton("Showing without DLCs and extras", QString(), ui->appliedFiltersHolder);
            checkbox = new QCheckBox("Hide DLCs and extras", ui->filtersScrollAreaContents);
            checkbox->setChecked(!filter.productTypes.contains("dlc"));
            if (checkbox->isChecked())
            {
                activatedFilterCount++;
                clearAllFiltersButton->setVisible(activatedFilterCount > 1);
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
            connect(checkbox, &QCheckBox::toggled,
                    this, [this, clearFilterButton, clearAllFiltersButton](bool value)
            {
                if (value)
                {
                    activatedFilterCount++;
                    clearAllFiltersButton->setVisible(activatedFilterCount > 1);
                    filter.productTypes = QStringList({"game","pack"});
                    ui->appliedFiltersHolder->layout()->addWidget(clearFilterButton);
                    clearFilterButton->setVisible(true);
                }
                else
                {
                    activatedFilterCount--;
                    clearAllFiltersButton->setVisible(activatedFilterCount > 1);
                    filter.productTypes = QStringList({"game","pack","dlc","extras"});
                    ui->appliedFiltersHolder->layout()->removeWidget(clearFilterButton);
                    clearFilterButton->setVisible(false);
                }

                if (applyFilters)
                {
                    page = 1;
                    fetchData();
                }
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
            connect(checkbox, &QCheckBox::toggled,
                    this, [this, clearFilterButton, clearAllFiltersButton](bool value)
            {
                if (value)
                {
                    activatedFilterCount++;
                    clearAllFiltersButton->setVisible(activatedFilterCount > 1);
                    ui->appliedFiltersHolder->layout()->addWidget(clearFilterButton);
                    clearFilterButton->setVisible(true);
                }
                else
                {
                    activatedFilterCount--;
                    clearAllFiltersButton->setVisible(activatedFilterCount > 1);
                    ui->appliedFiltersHolder->layout()->removeWidget(clearFilterButton);
                    clearFilterButton->setVisible(false);
                }
                filter.hideOwned = value;

                if (applyFilters)
                {
                    page = 1;
                    fetchData();
                }
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
            connect(checkbox, &QCheckBox::toggled,
                    this, [this, clearFilterButton, clearAllFiltersButton](bool value)
            {
                if (value)
                {
                    activatedFilterCount++;
                    clearAllFiltersButton->setVisible(activatedFilterCount > 1);
                    ui->appliedFiltersHolder->layout()->addWidget(clearFilterButton);
                    clearFilterButton->setVisible(true);
                }
                else
                {
                    activatedFilterCount--;
                    clearAllFiltersButton->setVisible(activatedFilterCount > 1);
                    ui->appliedFiltersHolder->layout()->removeWidget(clearFilterButton);
                    clearFilterButton->setVisible(false);
                }
                filter.onlyWishlisted = value;

                if (applyFilters)
                {
                    page = 1;
                    fetchData();
                }
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
            connect(checkbox, &QCheckBox::toggled,
                    this, [this, clearFilterButton, clearAllFiltersButton](bool value)
            {
                if (value)
                {
                    activatedFilterCount++;
                    clearAllFiltersButton->setVisible(activatedFilterCount > 1);
                    ui->appliedFiltersHolder->layout()->addWidget(clearFilterButton);
                    clearFilterButton->setVisible(true);
                }
                else
                {
                    activatedFilterCount--;
                    clearAllFiltersButton->setVisible(activatedFilterCount > 1);
                    ui->appliedFiltersHolder->layout()->removeWidget(clearFilterButton);
                    clearFilterButton->setVisible(false);
                }
                filter.free = value;

                if (applyFilters)
                {
                    page = 1;
                    fetchData();
                }
            });
            layout->addWidget(checkbox);
            area->setContentLayout(layout);
            maxWidth = std::max(maxWidth, layout->sizeHint().width());
            ui->filtersScrollAreaLayout->addWidget(area);

            area = new CollapsibleArea("Release Status", ui->filtersScrollAreaContents);
            area->setChangedFilters(filter.releaseStatuses.count() + filter.excludeReleaseStatuses.count());
            connect(area, &CollapsibleArea::clearFilters, this, [this, area]()
            {
                applyFilters = false;
                for (std::size_t i = 0; i < area->contentLayout()->count(); i++)
                {
                    auto filter = static_cast<FilterCheckbox*>(area->contentLayout()->itemAt(i)->widget());
                    filter->setInclude(false);
                    filter->setExclude(false);
                }
                applyFilters = true;

                page = 1;
                fetchData();
            });
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
                    clearAllFiltersButton->setVisible(activatedFilterCount > 1);
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
                    clearAllFiltersButton->setVisible(activatedFilterCount > 1);
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
                connect(filterCheckbox, &FilterCheckbox::include,
                        this, [this, item, area, clearFilterButton, clearAllFiltersButton](bool shouldInclude)
                {
                    if (shouldInclude)
                    {
                        activatedFilterCount++;
                        clearAllFiltersButton->setVisible(activatedFilterCount > 1);
                        filter.releaseStatuses.append(item.slug);
                        ui->appliedFiltersHolder->layout()->addWidget(clearFilterButton);
                        clearFilterButton->setVisible(true);
                    }
                    else
                    {
                        activatedFilterCount--;
                        clearAllFiltersButton->setVisible(activatedFilterCount > 1);
                        filter.releaseStatuses.removeOne(item.slug);
                        ui->appliedFiltersHolder->layout()->removeWidget(clearFilterButton);
                        clearFilterButton->setVisible(false);
                    }

                    area->setChangedFilters(filter.releaseStatuses.count() + filter.excludeReleaseStatuses.count());
                    if (applyFilters)
                    {
                        page = 1;
                        fetchData();
                    }
                });
                connect(clearHideFilterButton, &ClearFilterButton::clicked, filterCheckbox, [this, filterCheckbox]()
                {
                   filterCheckbox->setExclude(false);
                });
                connect(filterCheckbox, &FilterCheckbox::exclude,
                        this, [this, item, area, clearHideFilterButton, clearAllFiltersButton](bool shouldExclude)
                {
                    if (shouldExclude)
                    {
                        activatedFilterCount++;
                        clearAllFiltersButton->setVisible(activatedFilterCount > 1);
                        filter.excludeReleaseStatuses.append(item.slug);
                        ui->appliedFiltersHolder->layout()->addWidget(clearHideFilterButton);
                        clearHideFilterButton->setVisible(true);
                    }
                    else
                    {
                        activatedFilterCount--;
                        clearAllFiltersButton->setVisible(activatedFilterCount > 1);
                        filter.excludeReleaseStatuses.removeOne(item.slug);
                        ui->appliedFiltersHolder->layout()->removeWidget(clearHideFilterButton);
                        clearHideFilterButton->setVisible(false);
                    }

                    area->setChangedFilters(filter.releaseStatuses.count() + filter.excludeReleaseStatuses.count());
                    if (applyFilters)
                    {
                        page = 1;
                        fetchData();
                    }
                });
                layout->addWidget(filterCheckbox);
            }
            area->setContentLayout(layout);
            maxWidth = std::max(maxWidth, layout->sizeHint().width());
            ui->filtersScrollAreaLayout->addWidget(area);

            area = new CollapsibleArea("Genres", ui->filtersScrollAreaContents);
            area->setChangedFilters(filter.genres.count() + filter.excludeGenres.count());
            connect(area, &CollapsibleArea::clearFilters, this, [this, area]()
            {
                applyFilters = false;
                for (std::size_t i = 0; i < area->contentLayout()->count(); i++)
                {
                    auto filter = static_cast<FilterCheckbox*>(area->contentLayout()->itemAt(i)->widget());
                    filter->setInclude(false);
                    filter->setExclude(false);
                }
                applyFilters = true;

                page = 1;
                fetchData();
            });
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
                    clearAllFiltersButton->setVisible(activatedFilterCount > 1);
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
                    clearAllFiltersButton->setVisible(activatedFilterCount > 1);
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
                connect(filterCheckbox, &FilterCheckbox::include,
                        this, [this, item, area, clearFilterButton, clearAllFiltersButton](bool shouldInclude)
                {
                    if (shouldInclude)
                    {
                        activatedFilterCount++;
                        clearAllFiltersButton->setVisible(activatedFilterCount > 1);
                        filter.genres.append(item.slug);
                        ui->appliedFiltersHolder->layout()->addWidget(clearFilterButton);
                        clearFilterButton->setVisible(true);
                    }
                    else
                    {
                        activatedFilterCount--;
                        clearAllFiltersButton->setVisible(activatedFilterCount > 1);
                        filter.genres.removeOne(item.slug);
                        ui->appliedFiltersHolder->layout()->removeWidget(clearFilterButton);
                        clearFilterButton->setVisible(false);
                    }

                    area->setChangedFilters(filter.genres.count() + filter.excludeGenres.count());
                    if (applyFilters)
                    {
                        page = 1;
                        fetchData();
                    }
                });
                connect(clearHideFilterButton, &ClearFilterButton::clicked, filterCheckbox, [this, filterCheckbox]()
                {
                   filterCheckbox->setExclude(false);
                });
                connect(filterCheckbox, &FilterCheckbox::exclude,
                        this, [this, item, area, clearHideFilterButton, clearAllFiltersButton](bool shouldExclude)
                {
                    if (shouldExclude)
                    {
                        activatedFilterCount++;
                        clearAllFiltersButton->setVisible(activatedFilterCount > 1);
                        filter.excludeGenres.append(item.slug);
                        ui->appliedFiltersHolder->layout()->addWidget(clearHideFilterButton);
                        clearHideFilterButton->setVisible(true);
                    }
                    else
                    {
                        activatedFilterCount--;
                        clearAllFiltersButton->setVisible(activatedFilterCount > 1);
                        filter.excludeGenres.removeOne(item.slug);
                        ui->appliedFiltersHolder->layout()->removeWidget(clearHideFilterButton);
                        clearHideFilterButton->setVisible(false);
                    }

                    area->setChangedFilters(filter.genres.count() + filter.excludeGenres.count());
                    if (applyFilters)
                    {
                        page = 1;
                        fetchData();
                    }
                });
                layout->addWidget(filterCheckbox);
            }
            area->setContentLayout(layout);
            maxWidth = std::max(maxWidth, layout->sizeHint().width());
            ui->filtersScrollAreaLayout->addWidget(area);

            area = new CollapsibleArea("Tags", ui->filtersScrollAreaContents);
            area->setChangedFilters(filter.tags.count() + filter.excludeTags.count());
            connect(area, &CollapsibleArea::clearFilters, this, [this, area]()
            {
                applyFilters = false;
                for (std::size_t i = 0; i < area->contentLayout()->count(); i++)
                {
                    auto filter = static_cast<FilterCheckbox*>(area->contentLayout()->itemAt(i)->widget());
                    filter->setInclude(false);
                    filter->setExclude(false);
                }
                applyFilters = true;

                page = 1;
                fetchData();
            });
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
                    clearAllFiltersButton->setVisible(activatedFilterCount > 1);
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
                    clearAllFiltersButton->setVisible(activatedFilterCount > 1);
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
                connect(filterCheckbox, &FilterCheckbox::include,
                        this, [this, item, area, clearFilterButton, clearAllFiltersButton](bool shouldInclude)
                {
                    if (shouldInclude)
                    {
                        activatedFilterCount++;
                        clearAllFiltersButton->setVisible(activatedFilterCount > 1);
                        filter.tags.append(item.slug);
                        ui->appliedFiltersHolder->layout()->addWidget(clearFilterButton);
                        clearFilterButton->setVisible(true);
                    }
                    else
                    {
                        activatedFilterCount--;
                        clearAllFiltersButton->setVisible(activatedFilterCount > 1);
                        filter.tags.removeOne(item.slug);
                        ui->appliedFiltersHolder->layout()->removeWidget(clearFilterButton);
                        clearFilterButton->setVisible(false);
                    }

                    area->setChangedFilters(filter.tags.count() + filter.excludeTags.count());
                    if (applyFilters)
                    {
                        page = 1;
                        fetchData();
                    }
                });
                connect(clearHideFilterButton, &ClearFilterButton::clicked, filterCheckbox, [this, filterCheckbox]()
                {
                   filterCheckbox->setExclude(false);
                });
                connect(filterCheckbox, &FilterCheckbox::exclude,
                        this, [this, item, area, clearHideFilterButton, clearAllFiltersButton](bool shouldExclude)
                {
                    if (shouldExclude)
                    {
                        activatedFilterCount++;
                        clearAllFiltersButton->setVisible(activatedFilterCount > 1);
                        filter.excludeTags.append(item.slug);
                        ui->appliedFiltersHolder->layout()->addWidget(clearHideFilterButton);
                        clearHideFilterButton->setVisible(true);
                    }
                    else
                    {
                        activatedFilterCount--;
                        clearAllFiltersButton->setVisible(activatedFilterCount > 1);
                        filter.excludeTags.removeOne(item.slug);
                        ui->appliedFiltersHolder->layout()->removeWidget(clearHideFilterButton);
                        clearHideFilterButton->setVisible(false);
                    }

                    area->setChangedFilters(filter.tags.count() + filter.excludeTags.count());
                    if (applyFilters)
                    {
                        page = 1;
                        fetchData();
                    }
                });
                layout->addWidget(filterCheckbox);
            }
            layout->setAlignment(Qt::AlignTop);
            area->setContentLayout(layout);
            maxWidth = std::max(maxWidth, layout->sizeHint().width());
            ui->filtersScrollAreaLayout->addWidget(area);

            area = new CollapsibleArea("Operating Systems", ui->filtersScrollAreaContents);
            area->setChangedFilters(filter.systems.count());
            connect(area, &CollapsibleArea::clearFilters, this, [this, area]()
            {
                applyFilters = false;
                for (std::size_t i = 0; i < area->contentLayout()->count(); i++)
                {
                    auto filter = static_cast<QCheckBox*>(area->contentLayout()->itemAt(i)->widget());
                    filter->setChecked(false);
                }
                applyFilters = true;

                page = 1;
                fetchData();
            });
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
                    clearAllFiltersButton->setVisible(activatedFilterCount > 1);
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
                connect(checkbox, &QCheckBox::toggled,
                        this, [this, item, area, clearFilterButton, clearAllFiltersButton](bool shouldInclude)
                {
                    if (shouldInclude)
                    {
                        activatedFilterCount++;
                        clearAllFiltersButton->setVisible(activatedFilterCount > 1);
                        ui->appliedFiltersHolder->layout()->addWidget(clearFilterButton);
                        clearFilterButton->setVisible(true);
                        filter.systems.append(item.slug);
                    }
                    else
                    {
                        activatedFilterCount--;
                        clearAllFiltersButton->setVisible(activatedFilterCount > 1);
                        filter.systems.removeOne(item.slug);
                        ui->appliedFiltersHolder->layout()->removeWidget(clearFilterButton);
                        clearFilterButton->setVisible(false);
                    }

                    area->setChangedFilters(filter.systems.count());
                    if (applyFilters)
                    {
                        page = 1;
                        fetchData();
                    }
                });
                layout->addWidget(checkbox);
            }
            area->setContentLayout(layout);
            maxWidth = std::max(maxWidth, layout->sizeHint().width());
            ui->filtersScrollAreaLayout->addWidget(area);

            area = new CollapsibleArea("Features", ui->filtersScrollAreaContents);
            area->setChangedFilters(filter.features.count() + filter.excludeFeatures.count());
            connect(area, &CollapsibleArea::clearFilters, this, [this, area]()
            {
                applyFilters = false;
                for (std::size_t i = 0; i < area->contentLayout()->count(); i++)
                {
                    auto filter = static_cast<FilterCheckbox*>(area->contentLayout()->itemAt(i)->widget());
                    filter->setInclude(false);
                    filter->setExclude(false);
                }
                applyFilters = true;

                page = 1;
                fetchData();
            });
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
                    clearAllFiltersButton->setVisible(activatedFilterCount > 1);
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
                    clearAllFiltersButton->setVisible(activatedFilterCount > 1);
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
                connect(filterCheckbox, &FilterCheckbox::include,
                        this, [this, item, area, clearFilterButton, clearAllFiltersButton](bool shouldInclude)
                {
                    if (shouldInclude)
                    {
                        activatedFilterCount++;
                        clearAllFiltersButton->setVisible(activatedFilterCount > 1);
                        filter.features.append(item.slug);
                        ui->appliedFiltersHolder->layout()->addWidget(clearFilterButton);
                        clearFilterButton->setVisible(true);
                    }
                    else
                    {
                        activatedFilterCount--;
                        clearAllFiltersButton->setVisible(activatedFilterCount > 1);
                        filter.features.removeOne(item.slug);
                        ui->appliedFiltersHolder->layout()->removeWidget(clearFilterButton);
                        clearFilterButton->setVisible(false);
                    }

                    area->setChangedFilters(filter.features.count() + filter.excludeFeatures.count());
                    if (applyFilters)
                    {
                        page = 1;
                        fetchData();
                    }
                });
                connect(clearHideFilterButton, &ClearFilterButton::clicked, filterCheckbox, [this, filterCheckbox]()
                {
                   filterCheckbox->setExclude(false);
                });
                connect(filterCheckbox, &FilterCheckbox::exclude,
                        this, [this, item, area, clearHideFilterButton, clearAllFiltersButton](bool shouldExclude)
                {
                    if (shouldExclude)
                    {
                        activatedFilterCount++;
                        clearAllFiltersButton->setVisible(activatedFilterCount > 1);
                        filter.excludeFeatures.append(item.slug);
                        ui->appliedFiltersHolder->layout()->addWidget(clearHideFilterButton);
                        clearHideFilterButton->setVisible(true);
                    }
                    else
                    {
                        activatedFilterCount--;
                        clearAllFiltersButton->setVisible(activatedFilterCount > 1);
                        filter.excludeFeatures.removeOne(item.slug);
                        ui->appliedFiltersHolder->layout()->removeWidget(clearHideFilterButton);
                        clearHideFilterButton->setVisible(false);
                    }

                    area->setChangedFilters(filter.features.count() + filter.excludeFeatures.count());
                    if (applyFilters)
                    {
                        page = 1;
                        fetchData();
                    }
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
            connect(area, &CollapsibleArea::clearFilters, this, [this, area]()
            {
                applyFilters = false;
                for (std::size_t i = 0; i < area->contentLayout()->count(); i++)
                {
                    auto filter = static_cast<QCheckBox*>(area->contentLayout()->itemAt(i)->widget());
                    filter->setChecked(false);
                }
                applyFilters = true;

                page = 1;
                fetchData();
            });
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
                    clearAllFiltersButton->setVisible(activatedFilterCount > 1);
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
                connect(checkbox, &QCheckBox::toggled,
                        this, [this, item, area, clearFilterButton, clearAllFiltersButton](bool shouldInclude)
                {
                    if (shouldInclude)
                    {
                        activatedFilterCount++;
                        clearAllFiltersButton->setVisible(activatedFilterCount > 1);
                        filter.languages.append(item.slug);
                        ui->appliedFiltersHolder->layout()->addWidget(clearFilterButton);
                        clearFilterButton->setVisible(true);
                    }
                    else
                    {
                        activatedFilterCount--;
                        clearAllFiltersButton->setVisible(activatedFilterCount > 1);
                        filter.languages.removeOne(item.slug);
                        ui->appliedFiltersHolder->layout()->removeWidget(clearFilterButton);
                        clearFilterButton->setVisible(false);
                    }

                    area->setChangedFilters(filter.languages.count());
                    if (applyFilters)
                    {
                        page = 1;
                        fetchData();
                    }
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

