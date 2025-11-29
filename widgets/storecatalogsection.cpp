#include "storecatalogsection.h"
#include "ui_storecatalogsection.h"

#include <algorithm>

#include <QCheckBox>
#include <QJsonDocument>
#include <QJsonObject>
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

StoreCatalogSection::StoreCatalogSection(QWidget *parent) :
    QWidget(parent),
    activatedFilterCount(0),
    applyFilters(true),
    filter({}),
    ui(new Ui::StoreCatalogSection)
{
    ui->setupUi(this);
    gridLayout = true;

    currentSortOrder = 0;
    orders =
    {
        api::SortOrder{QLatin1StringView("trending"), false},
        api::SortOrder{QLatin1StringView("bestselling"), false},
        api::SortOrder{QLatin1StringView("price"), true},
        api::SortOrder{QLatin1StringView("price"), false},
        api::SortOrder{QLatin1StringView("discount"), false},
        api::SortOrder{QLatin1StringView("title"), true},
        api::SortOrder{QLatin1StringView("title"), false},
        api::SortOrder{QLatin1StringView("releaseDate"), false},
        api::SortOrder{QLatin1StringView("releaseDate"), true},
        api::SortOrder{QLatin1StringView("reviewsRating"), false}
    };

    filter.goodOldGames = false;
    filter.free = false;
    filter.discounted = false;
    filter.hideOwned = false;
    filter.onlyWishlisted = false;
    filter.productTypes = QStringList(
    {
        QLatin1StringView("game"),
        QLatin1StringView("pack"),
        QLatin1StringView("dlc"),
        QLatin1StringView("extras")
    });

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

    connect(ui->lineEdit, &QLineEdit::textChanged, this, [this](const QString &query)
    {
        page = 1;
        filter.query = query.trimmed();
        fetchData();
    });
    connect(ui->sortOrderComboBox, &QComboBox::currentIndexChanged, this, [this](int index)
    {
        if (index < 0 || index > 9)
        {
            index = 0;
        }
        page = 1;
        currentSortOrder = index;
        fetchData();
    });
    connect(ui->gridModeButton, &QPushButton::clicked, this, [this]()
    {
        if (!gridLayout)
        {
            gridLayout = true;
            while (!listResultsPage->layout()->isEmpty())
            {
                auto item = listResultsPage->layout()->itemAt(0);
                listResultsPage->layout()->removeItem(item);
                item->widget()->deleteLater();
                delete item;
            }
            layoutResults();
        }
    });
    connect(ui->listModeButton, &QPushButton::clicked, this, [this]()
    {
        if (gridLayout)
        {
            gridLayout = false;
            while (!gridResultsPage->layout()->isEmpty())
            {
                auto item = gridResultsPage->layout()->itemAt(0);
                gridResultsPage->layout()->removeItem(item);
                item->widget()->deleteLater();
                delete item;
            }
            layoutResults();
        }
    });
}

StoreCatalogSection::~StoreCatalogSection()
{
    delete ui;
}

void StoreCatalogSection::fetchData()
{
    emit updatingData();
    ui->contentsStack->setCurrentWidget(ui->loaderPage);
    data.products.clear();
    while (!listResultsPage->layout()->isEmpty())
    {
        auto item = listResultsPage->layout()->itemAt(0);
        listResultsPage->layout()->removeItem(item);
        item->widget()->deleteLater();
        delete item;
    }
    while (!gridResultsPage->layout()->isEmpty())
    {
        auto item = gridResultsPage->layout()->itemAt(0);
        gridResultsPage->layout()->removeItem(item);
        item->widget()->deleteLater();
        delete item;
    }

    auto systemLocale = QLocale::system();
    QNetworkReply *catalogReply = apiClient->searchCatalog(orders[currentSortOrder], filter,
                                                QLocale::territoryToCode(systemLocale.territory()),
                                                systemLocale.name(QLocale::TagSeparator::Dash),
                                                systemLocale.currencySymbol(QLocale::CurrencyIsoCode), page);
    connect(this, &QObject::destroyed, catalogReply, &QNetworkReply::abort);
    connect(this, &StoreCatalogSection::updatingData, catalogReply, &QNetworkReply::abort);
    connect(catalogReply, &QNetworkReply::finished, this, [this, catalogReply](){
        if (catalogReply->error() == QNetworkReply::NoError)
        {
            auto resultJson = QJsonDocument::fromJson(QString(catalogReply->readAll()).toUtf8()).object();
            parseSearchCatalogResponse(resultJson, data, QLatin1StringView("_product_tile_extended_432x243.webp"));

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
        else if (catalogReply->error() != QNetworkReply::OperationCanceledError)
        {   ui->contentsStack->setCurrentWidget(ui->errorPage);
            qDebug() << catalogReply->error() << catalogReply->errorString() << QString(catalogReply->readAll()).toUtf8();
        }
    });
    connect(catalogReply, &QNetworkReply::finished, catalogReply, &QNetworkReply::deleteLater);
}

void StoreCatalogSection::layoutResults()
{
    ui->contentsStack->setCurrentWidget(ui->loaderPage);
    if (gridLayout)
    {
        for (const api::CatalogProduct &product : std::as_const(data.products))
        {
            auto storeItem = new StoreGridTile(product, apiClient, gridResultsPage);
            connect(this, &StoreCatalogSection::ownedProductsChanged,
                    storeItem, [storeItem, productId = product.id](const QSet<const QString> &ids)
            {
                storeItem->setOwned(ids.contains(productId));
            });
            storeItem->setOwned(ownedProducts.contains(product.id));
            connect(this, &StoreCatalogSection::wishlistChanged,
                    storeItem, [storeItem, productId = product.id](const QSet<const QString> &ids)
            {
                storeItem->setWishlisted(ids.contains(productId));
            });
            storeItem->setWishlisted(wishlist.contains(product.id));
            connect(apiClient, &api::GogApiClient::authenticated, storeItem,
                    &StoreGridTile::switchUiAuthenticatedState);
            storeItem->switchUiAuthenticatedState(apiClient->isAuthenticated());
            connect(storeItem, &StoreGridTile::clicked, this, [this, productId = product.id]()
            {
                emit navigate({Page::CATALOG_PRODUCT, productId});
            });
            gridResultsPage->layout()->addWidget(storeItem);
        }
        ui->resultsStackedWidget->setCurrentWidget(ui->resultsGridPage);
    }
    else
    {
        for (const api::CatalogProduct &product : std::as_const(data.products))
        {
            auto storeItem = new StoreListItem(product, apiClient, listResultsPage);
            connect(this, &StoreCatalogSection::ownedProductsChanged,
                    storeItem, [storeItem, productId = product.id](const QSet<const QString> &ids)
            {
                storeItem->setOwned(ids.contains(productId));
            });
            storeItem->setOwned(ownedProducts.contains(product.id));
            connect(this, &StoreCatalogSection::wishlistChanged,
                    storeItem, [storeItem, productId = product.id](const QSet<const QString> &ids)
            {
                storeItem->setWishlisted(ids.contains(productId));
            });
            storeItem->setWishlisted(wishlist.contains(product.id));
            connect(apiClient, &api::GogApiClient::authenticated, storeItem,
                    &StoreListItem::switchUiAuthenticatedState);
            storeItem->switchUiAuthenticatedState(apiClient->isAuthenticated());
            connect(storeItem, &StoreListItem::clicked, this, [this, productId = product.id]()
            {
                emit navigate({Page::CATALOG_PRODUCT, productId});
            });
            listResultsPage->layout()->addWidget(storeItem);
        }
        ui->resultsStackedWidget->setCurrentWidget(ui->resultsListPage);
    }
    ui->contentsStack->setCurrentWidget(ui->resultsPage);
}

void StoreCatalogSection::initialize(const QVariant &data, api::GogApiClient *apiClient, bool externalContentScroll)
{
    QMap<QString, QVariant> initialFilters = data.toMap();
    if (initialFilters.contains(QLatin1StringView("goodOldGames")))
    {
        filter.goodOldGames = initialFilters[QLatin1StringView("goodOldGames")].toBool();
    }
    if (initialFilters.contains(QLatin1StringView("discounted")))
    {
        filter.discounted = initialFilters[QLatin1StringView("discounted")].toBool();
    }
    if (initialFilters.contains(QLatin1StringView("developer")))
    {
        filter.developers << initialFilters[QLatin1StringView("developer")].toString();
    }
    if (initialFilters.contains(QLatin1StringView("publisher")))
    {
        filter.publishers << initialFilters[QLatin1StringView("publisher")].toString();
    }
    if (initialFilters.contains(QLatin1StringView("genre")))
    {
        filter.genres << initialFilters[QLatin1StringView("genre")].toString();
    }
    if (initialFilters.contains(QLatin1StringView("tag")))
    {
        filter.tags << initialFilters[QLatin1StringView("tag")].toString();
    }
    if (initialFilters.contains(QLatin1StringView("feature")))
    {
        filter.features << initialFilters[QLatin1StringView("feature")].toString();
    }
    if (initialFilters.contains(QLatin1StringView("releaseStatus")))
    {
        filter.releaseStatuses << initialFilters[QLatin1StringView("releaseStatus")].toString();
    }
    if (initialFilters.contains(QLatin1StringView("pageId")))
    {
        filter.pageId = initialFilters[QLatin1StringView("pageId")].toString();
    }
    if (initialFilters.contains(QLatin1StringView("sectionId")))
    {
        filter.sectionId = initialFilters[QLatin1StringView("sectionId")].toString();
    }

    this->apiClient = apiClient;

    if (externalContentScroll)
    {
        gridResultsPage = ui->resultsGridPage;
        gridResultsPage->setLayout(new FlowLayout(gridResultsPage, -1, 24, 24));
        listResultsPage = ui->resultsListPage;
        listResultsPage->setLayout(new QVBoxLayout(listResultsPage));
        listResultsPage->layout()->setContentsMargins(24, 0, 8, 0);
        listResultsPage->layout()->setSpacing(1);
        listResultsPage->layout()->setAlignment(Qt::AlignTop);
    }
    else
    {
        auto resultsGridScrollArea = new QScrollArea(ui->resultsGridPage);
        resultsGridScrollArea->setWidgetResizable(true);
        connect(this, &StoreCatalogSection::updatingData, resultsGridScrollArea, [resultsGridScrollArea]()
        {
            resultsGridScrollArea->verticalScrollBar()->setValue(0);
        });
        gridResultsPage = new QWidget(resultsGridScrollArea);
        gridResultsPage->setLayout(new FlowLayout(gridResultsPage, -1, 24, 24));
        resultsGridScrollArea->setWidget(gridResultsPage);
        ui->resultsGridPage->setLayout(new QVBoxLayout(ui->resultsGridPage));
        ui->resultsGridPage->layout()->setContentsMargins(0, 0, 0, 0);
        ui->resultsGridPage->layout()->addWidget(resultsGridScrollArea);

        auto resultsListScrollArea = new QScrollArea(ui->resultsListPage);
        resultsListScrollArea->setWidgetResizable(true);
        connect(this, &StoreCatalogSection::updatingData, resultsListScrollArea, [resultsListScrollArea]()
        {
            resultsListScrollArea->verticalScrollBar()->setValue(0);
        });
        listResultsPage = new QWidget(resultsListScrollArea);
        listResultsPage->setLayout(new QVBoxLayout(listResultsPage));
        listResultsPage->layout()->setContentsMargins(24, 0, 8, 0);
        listResultsPage->layout()->setSpacing(1);
        listResultsPage->layout()->setAlignment(Qt::AlignTop);
        resultsListScrollArea->setWidget(listResultsPage);
        ui->resultsListPage->setLayout(new QVBoxLayout(ui->resultsListPage));
        ui->resultsListPage->layout()->setContentsMargins(0, 0, 0, 0);
        ui->resultsListPage->layout()->addWidget(resultsListScrollArea);
    }

    auto systemLocale = QLocale::system();
    QNetworkReply *catalogReply = apiClient->searchCatalog(orders[currentSortOrder], filter,
                                                QLocale::territoryToCode(systemLocale.territory()),
                                                systemLocale.name(QLocale::TagSeparator::Dash),
                                                systemLocale.currencySymbol(QLocale::CurrencyIsoCode), page);
    connect(this, &QObject::destroyed, catalogReply, &QNetworkReply::abort);
    ui->filtersScrollArea->setVisible(false);
    ui->contentsStack->setCurrentWidget(ui->loaderPage);
    connect(catalogReply, &QNetworkReply::finished, this, [this, catalogReply](){
        if (catalogReply->error() == QNetworkReply::NoError)
        {
            auto resultJson = QJsonDocument::fromJson(QString(catalogReply->readAll()).toUtf8()).object();
            parseSearchCatalogResponse(resultJson, this->data, QLatin1StringView("_product_tile_extended_432x243.webp"));

            auto clearAllFiltersButton = new ClearFilterButton(tr("Clear all filters"), QString(), ui->appliedFiltersHolder);
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
                auto clearFilterButton = new ClearFilterButton(tr("Selected developer"), QString(), ui->appliedFiltersHolder);
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
                auto clearFilterButton = new ClearFilterButton(tr("Selected publisher"), QString(), ui->appliedFiltersHolder);
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

            clearFilterButton = new ClearFilterButton(tr("GOOD OLD GAMES"), QString(), ui->appliedFiltersHolder);
            checkbox = new QCheckBox(tr("GOOD OLD GAMES"), ui->filtersScrollAreaContents);
            maxWidth = std::max(maxWidth, checkbox->width());
            checkbox->setChecked(filter.goodOldGames);
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
                    ui->appliedFiltersHolder->layout()->addWidget(clearFilterButton);
                }
                else
                {
                    activatedFilterCount--;
                    ui->appliedFiltersHolder->layout()->removeWidget(clearFilterButton);
                }
                filter.goodOldGames = value;
                clearAllFiltersButton->setVisible(activatedFilterCount > 1);
                clearFilterButton->setVisible(value);

                if (applyFilters)
                {
                    page = 1;
                    fetchData();
                }
            });
            ui->filtersScrollAreaLayout->addWidget(checkbox);

            clearFilterButton = new ClearFilterButton(tr("Discounted"), QString(), ui->appliedFiltersHolder);
            checkbox = new QCheckBox(tr("Show only discounted"), ui->filtersScrollAreaContents);
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
                    ui->appliedFiltersHolder->layout()->addWidget(clearFilterButton);
                }
                else
                {
                    activatedFilterCount--;
                    ui->appliedFiltersHolder->layout()->removeWidget(clearFilterButton);
                }
                filter.discounted = value;
                clearAllFiltersButton->setVisible(activatedFilterCount > 1);
                clearFilterButton->setVisible(value);

                if (applyFilters)
                {
                    page = 1;
                    fetchData();
                }
            });
            ui->filtersScrollAreaLayout->addWidget(checkbox);

            clearFilterButton = new ClearFilterButton(tr("Excluding owned products"), QString(), ui->appliedFiltersHolder);
            checkbox = new QCheckBox(tr("Hide all owned products"), ui->filtersScrollAreaContents);
            maxWidth = std::max(maxWidth, checkbox->width());
            checkbox->setChecked(filter.hideOwned);
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
                    ui->appliedFiltersHolder->layout()->addWidget(clearFilterButton);
                }
                else
                {
                    activatedFilterCount--;
                    ui->appliedFiltersHolder->layout()->removeWidget(clearFilterButton);
                }
                filter.hideOwned = value;
                clearAllFiltersButton->setVisible(activatedFilterCount > 1);
                clearFilterButton->setVisible(value);

                if (applyFilters)
                {
                    page = 1;
                    fetchData();
                }
            });
            connect(this->apiClient, &api::GogApiClient::authenticated,
                    checkbox, [checkbox](bool authenticated)
            {
                checkbox->setVisible(authenticated);
                if (!authenticated)
                {
                    checkbox->setChecked(false);
                }
            });
            checkbox->setVisible(this->apiClient->isAuthenticated());
            ui->filtersScrollAreaLayout->addWidget(checkbox);

            clearFilterButton = new ClearFilterButton(tr("Showing only wishlisted games"), QString(), ui->appliedFiltersHolder);
            checkbox = new QCheckBox(tr("Show only games on my wishlist"), ui->filtersScrollAreaContents);
            maxWidth = std::max(maxWidth, checkbox->width());
            checkbox->setChecked(filter.onlyWishlisted);
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
                    ui->appliedFiltersHolder->layout()->addWidget(clearFilterButton);
                }
                else
                {
                    activatedFilterCount--;
                    ui->appliedFiltersHolder->layout()->removeWidget(clearFilterButton);
                }
                filter.onlyWishlisted = value;
                clearAllFiltersButton->setVisible(activatedFilterCount > 1);
                clearFilterButton->setVisible(value);

                if (applyFilters)
                {
                    page = 1;
                    fetchData();
                }
            });
            connect(this->apiClient, &api::GogApiClient::authenticated,
                    checkbox, [checkbox](bool authenticated)
            {
                checkbox->setVisible(authenticated);
                if (!authenticated)
                {
                    checkbox->setChecked(false);
                }
            });
            checkbox->setVisible(this->apiClient->isAuthenticated());
            ui->filtersScrollAreaLayout->addWidget(checkbox);

            area = new CollapsibleArea("DLCs", ui->filtersScrollAreaContents);
            layout = new QVBoxLayout();
            clearFilterButton = new ClearFilterButton(tr("Showing without DLCs and extras"), QString(), ui->appliedFiltersHolder);
            checkbox = new QCheckBox(tr("Hide DLCs and extras"), area);
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
                    filter.productTypes = QStringList({"game","pack"});
                    ui->appliedFiltersHolder->layout()->addWidget(clearFilterButton);
                }
                else
                {
                    activatedFilterCount--;
                    filter.productTypes = QStringList({"game","pack","dlc","extras"});
                    ui->appliedFiltersHolder->layout()->removeWidget(clearFilterButton);
                }
                clearAllFiltersButton->setVisible(activatedFilterCount > 1);
                clearFilterButton->setVisible(value);

                if (applyFilters)
                {
                    page = 1;
                    fetchData();
                }
            });
            layout->addWidget(checkbox);
            clearFilterButton = new ClearFilterButton(tr("Showing only DLCs for owned games"), QString(), ui->appliedFiltersHolder);
            checkbox = new QCheckBox(tr("Show only DLCs for my games"), area);
            checkbox->setChecked(filter.onlyDlcForOwned);
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
                    ui->appliedFiltersHolder->layout()->addWidget(clearFilterButton);
                }
                else
                {
                    activatedFilterCount--;
                    ui->appliedFiltersHolder->layout()->removeWidget(clearFilterButton);
                }
                filter.onlyDlcForOwned = value;
                clearFilterButton->setVisible(value);
                clearAllFiltersButton->setVisible(activatedFilterCount > 1);

                if (applyFilters)
                {
                    page = 1;
                    fetchData();
                }
            });
            connect(this->apiClient, &api::GogApiClient::authenticated,
                    checkbox, [checkbox](bool authenticated)
            {
                checkbox->setVisible(authenticated);
                if (!authenticated)
                {
                    checkbox->setChecked(false);
                }
            });
            checkbox->setVisible(this->apiClient->isAuthenticated());
            layout->addWidget(checkbox);
            area->setContentLayout(layout);
            maxWidth = std::max(maxWidth, layout->sizeHint().width());
            ui->filtersScrollAreaLayout->addWidget(area);

            clearFilterButton = new ClearFilterButton(tr("Excluding owned products"), QString(), ui->appliedFiltersHolder);
            clearFilterButton->setVisible(false);
            checkbox = new QCheckBox(tr("Hide all owned products"), ui->filtersScrollAreaContents);
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

            clearFilterButton = new ClearFilterButton(tr("Showing only wishlisted games"), QString(), ui->appliedFiltersHolder);
            clearFilterButton->setVisible(false);
            checkbox = new QCheckBox(tr("Show only games on my wishlist"), ui->filtersScrollAreaContents);
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

            area = new CollapsibleArea(tr("Price range"), ui->filtersScrollAreaContents);
            layout = new QVBoxLayout();
            clearFilterButton = new ClearFilterButton(tr("Price Range"), "0 - 0", ui->appliedFiltersHolder);
            clearFilterButton->setVisible(false);
            checkbox = new QCheckBox(tr("Show only free games"), area);
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

            area = new CollapsibleArea(tr("Release Status"), ui->filtersScrollAreaContents);
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
                auto clearFilterButton = new ClearFilterButton(tr("Release status"), item.name, ui->appliedFiltersHolder);
                auto clearHideFilterButton = new ClearFilterButton(tr("Hide release status"), item.name, ui->appliedFiltersHolder);
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
                        filter.releaseStatuses << item.slug;
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
                        filter.excludeReleaseStatuses << item.slug;
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
                auto clearFilterButton = new ClearFilterButton(tr("Genre"), item.name, ui->appliedFiltersHolder);
                auto clearHideFilterButton = new ClearFilterButton(tr("Hide genre"), item.name, ui->appliedFiltersHolder);
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
                        filter.genres << item.slug;
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
                        filter.excludeGenres << item.slug;
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

            area = new CollapsibleArea(tr("Tags"), ui->filtersScrollAreaContents);
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
            for (const api::MetaTag &item : std::as_const(this->data.filters.fullTagsList))
            {
                auto clearFilterButton = new ClearFilterButton(tr("Tag"), item.name, ui->appliedFiltersHolder);
                auto clearHideFilterButton = new ClearFilterButton(tr("Hide tag"), item.name, ui->appliedFiltersHolder);
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
                        filter.tags << item.slug;
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
                        filter.excludeTags << item.slug;
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

            area = new CollapsibleArea(tr("Operating Systems"), ui->filtersScrollAreaContents);
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
                clearFilterButton = new ClearFilterButton(tr("Operating System"), item.name, ui->appliedFiltersHolder);
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
                        filter.systems << item.slug;
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

            area = new CollapsibleArea(tr("Features"), ui->filtersScrollAreaContents);
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
                auto clearFilterButton = new ClearFilterButton(tr("Feature"), item.name, ui->appliedFiltersHolder);
                auto clearHideFilterButton = new ClearFilterButton(tr("Hide feature"), item.name, ui->appliedFiltersHolder);
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
                        filter.features << item.slug;
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
                        filter.excludeFeatures << item.slug;
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

            area = new CollapsibleArea(tr("Release Date"), ui->filtersScrollAreaContents);
            ui->filtersScrollAreaLayout->addWidget(area);

            area = new CollapsibleArea(tr("Languages"), ui->filtersScrollAreaContents);
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
                clearFilterButton = new ClearFilterButton(tr("Language"), item.name, ui->appliedFiltersHolder);
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
                        filter.languages << item.slug;
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
        else if (catalogReply->error() != QNetworkReply::OperationCanceledError)
        {   ui->contentsStack->setCurrentWidget(ui->errorPage);
            qDebug() << catalogReply->error() << catalogReply->errorString() << QString(catalogReply->readAll()).toUtf8();
        }
    });
    connect(catalogReply, &QNetworkReply::finished, catalogReply, &QNetworkReply::deleteLater);
}

void StoreCatalogSection::updateOwnedProducts(const QSet<const QString> &ids)
{
    ownedProducts = ids;
    emit ownedProductsChanged(ids);
}

void StoreCatalogSection::updateWishlist(const QSet<const QString> &ids)
{
    wishlist = ids;
    emit wishlistChanged(ids);
}
