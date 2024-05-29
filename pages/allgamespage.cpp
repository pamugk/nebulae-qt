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
#include "../widgets/collapsiblearea.h"
#include "../widgets/filtercheckbox.h"
#include "../widgets/storegridtile.h"
#include "../widgets/storelistitem.h"

AllGamesPage::AllGamesPage(QWidget *parent) :
    BasePage(parent),
    lastCatalogReply(nullptr),
    ui(new Ui::AllGamesPage)
{
    ui->setupUi(this);
    ui->filtersScrollAreaLayout->setAlignment(Qt::AlignTop);
    ui->resultsScrollAreaLayout->setAlignment(Qt::AlignTop);
    ui->resultsGridPage->setLayout(new FlowLayout(ui->resultsGridPage));
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

    filter.discounted = false;
    filter.hideOwned = false;
    filter.productTypes = QStringList({"game","pack","dlc","extras"});

    page = 1;
    paginator = new Pagination(ui->resultsPage);
    connect(paginator, &Pagination::changedPage, this, [this](quint16 newPage)
    {
       page = newPage;
       fetchData();
    });
    ui->resultsPageLayout->addWidget(paginator);
    ui->resultsPageLayout->setAlignment(paginator, Qt::AlignHCenter);
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
    paginator->setVisible(false);
    data.products.clear();
    while (!ui->resultsListLayout->isEmpty())
    {
        auto item = ui->resultsListLayout->itemAt(0);
        ui->resultsListLayout->removeItem(item);
        item->widget()->deleteLater();
        delete item;
    }
    while (!ui->resultsGridPage->layout()->isEmpty())
    {
        auto item = ui->resultsGridPage->layout()->itemAt(0);
        ui->resultsGridPage->layout()->removeItem(item);
        item->widget()->deleteLater();
        delete item;
    }

    if (lastCatalogReply != nullptr)
    {
        lastCatalogReply->abort();
    }
    lastCatalogReply = apiClient->searchCatalog(orders[currentSortOrder], filter, "RU", "en-US", "RUB", page);
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
                ui->contentsStack->setCurrentWidget(ui->emptyPage);
            }
            else
            {
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
        foreach (api::CatalogProduct product, data.products)
        {
            auto storeItem = new StoreGridTile(product, apiClient, ui->resultsListPage);
            connect(storeItem, &StoreGridTile::navigateToProduct, this, [this](quint64 productId)
            {
                emit navigate({Page::CATALOG_PRODUCT_PAGE, productId});
            });
            ui->resultsGridPage->layout()->addWidget(storeItem);
        }
        ui->resultsStackedWidget->setCurrentWidget(ui->resultsGridPage);
    }
    else
    {
        foreach (api::CatalogProduct product, data.products)
        {
            auto storeItem = new StoreListItem(product, apiClient, ui->resultsListPage);
            connect(storeItem, &StoreListItem::navigateToProduct, this, [this](quint64 productId)
            {
                emit navigate({Page::CATALOG_PRODUCT_PAGE, productId});
            });
            ui->resultsListLayout->addWidget(storeItem);
        }
        ui->resultsStackedWidget->setCurrentWidget(ui->resultsListPage);
    }
    ui->contentsStack->setCurrentWidget(ui->resultsPage);
}

void AllGamesPage::initialize(const QVariant &data)
{
    auto networkReply = apiClient->searchCatalog(orders[currentSortOrder], filter, "RU", "en-US", "RUB", page);
    ui->filtersScrollArea->setVisible(false);
    ui->contentsStack->setCurrentWidget(ui->loaderPage);
    connect(networkReply, &QNetworkReply::finished, this, [=](){
        if (networkReply->error() == QNetworkReply::NoError)
        {
            auto resultJson = QJsonDocument::fromJson(QString(networkReply->readAll()).toUtf8()).object();
            parseSearchCatalogResponse(resultJson, this->data);

            int maxWidth = 0;
            CollapsibleArea *area;
            QLayout *layout;
            QCheckBox *checkbox;

            checkbox = new QCheckBox("Show only discounted", ui->filtersScrollAreaContents);
            maxWidth = std::max(maxWidth, checkbox->width());
            checkbox->setChecked(filter.discounted);
            connect(checkbox, &QCheckBox::toggled, this, [this](bool value)
            {
                filter.discounted = value;
                page = 1;
                fetchData();
            });
            ui->filtersScrollAreaLayout->addWidget(checkbox);

            checkbox = new QCheckBox("Hide DLCs and extras", ui->filtersScrollAreaContents);
            checkbox->setChecked(!filter.productTypes.contains("dlc"));
            maxWidth = std::max(maxWidth, checkbox->width());
            connect(checkbox, &QCheckBox::toggled, this, [this](bool value)
            {
                filter.productTypes = value ? QStringList({"game","pack"}) : QStringList({"game","pack","dlc","extras"});
                page = 1;
                fetchData();
            });
            ui->filtersScrollAreaLayout->addWidget(checkbox);

            checkbox = new QCheckBox("Hide all owned products", ui->filtersScrollAreaContents);
            checkbox->setChecked(filter.hideOwned);
            connect(checkbox, &QCheckBox::toggled, this, [this](bool value)
            {
                filter.hideOwned = value;
                page = 1;
                fetchData();
            });
            ui->filtersScrollAreaLayout->addWidget(checkbox);

            area = new CollapsibleArea("Price range", ui->filtersScrollAreaContents);
            layout = new QVBoxLayout(area);
            checkbox = new QCheckBox("Show only free games", area);
            layout->setAlignment(Qt::AlignTop);
            layout->addWidget(checkbox);
            area->setContentLayout(layout);
            maxWidth = std::max(maxWidth, layout->sizeHint().width());
            ui->filtersScrollAreaLayout->addWidget(area);

            area = new CollapsibleArea("Release Status", ui->filtersScrollAreaContents);
            area->setChangedFilters(filter.releaseStatuses.count() + filter.excludeReleaseStatuses.count());
            layout = new QVBoxLayout(area);
            layout->setAlignment(Qt::AlignTop);
            foreach (api::MetaTag item, this->data.filters.releaseStatuses)
            {
                auto filterCheckbox = new FilterCheckbox(item.name, area);
                filterCheckbox->setInclude(filter.releaseStatuses.contains(item.slug));
                filterCheckbox->setExclude(filter.excludeReleaseStatuses.contains(item.slug));
                connect(filterCheckbox, &FilterCheckbox::include, this, [this, item, area](bool shouldInclude)
                {
                    if (shouldInclude)
                    {
                        filter.releaseStatuses.append(item.slug);
                    }
                    else
                    {
                        filter.releaseStatuses.removeOne(item.slug);
                    }
                    page = 1;
                    area->setChangedFilters(filter.releaseStatuses.count() + filter.excludeReleaseStatuses.count());
                    fetchData();
                });
                connect(filterCheckbox, &FilterCheckbox::exclude, this, [this, item, area](bool shouldExclude)
                {
                    if (shouldExclude)
                    {
                        filter.excludeReleaseStatuses.append(item.slug);
                    }
                    else
                    {
                        filter.excludeReleaseStatuses.removeOne(item.slug);
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
            layout = new QVBoxLayout(area);
            layout->setAlignment(Qt::AlignTop);
            foreach (api::MetaTag item, this->data.filters.genres)
            {
                auto filterCheckbox = new FilterCheckbox(item.name, area);
                filterCheckbox->setInclude(filter.genres.contains(item.slug));
                filterCheckbox->setExclude(filter.excludeGenres.contains(item.slug));
                connect(filterCheckbox, &FilterCheckbox::include, this, [this, item, area](bool shouldInclude)
                {
                    if (shouldInclude)
                    {
                        filter.genres.append(item.slug);
                    }
                    else
                    {
                        filter.genres.removeOne(item.slug);
                    }
                    page = 1;
                    area->setChangedFilters(filter.genres.count() + filter.excludeGenres.count());
                    fetchData();
                });
                connect(filterCheckbox, &FilterCheckbox::exclude, this, [this, item, area](bool shouldExclude)
                {
                    if (shouldExclude)
                    {
                        filter.excludeGenres.append(item.slug);
                    }
                    else
                    {
                        filter.excludeGenres.removeOne(item.slug);
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
            layout = new QVBoxLayout(area);
            layout->setAlignment(Qt::AlignTop);
            foreach (api::MetaTag item, this->data.filters.tags)
            {
                auto filterCheckbox = new FilterCheckbox(item.name, area);
                filterCheckbox->setInclude(filter.tags.contains(item.slug));
                filterCheckbox->setExclude(filter.excludeTags.contains(item.slug));
                connect(filterCheckbox, &FilterCheckbox::include, this, [this, item, area](bool shouldInclude)
                {
                    if (shouldInclude)
                    {
                        filter.tags.append(item.slug);
                    }
                    else
                    {
                        filter.tags.removeOne(item.slug);
                    }
                    page = 1;
                    area->setChangedFilters(filter.tags.count() + filter.excludeTags.count());
                    fetchData();
                });
                connect(filterCheckbox, &FilterCheckbox::exclude, this, [this, item, area](bool shouldExclude)
                {
                    if (shouldExclude)
                    {
                        filter.excludeTags.append(item.slug);
                    }
                    else
                    {
                        filter.excludeTags.removeOne(item.slug);
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
            layout = new QVBoxLayout(area);
            layout->setAlignment(Qt::AlignTop);
            foreach (api::MetaTag item, this->data.filters.systems)
            {
                auto checkbox = new QCheckBox(item.name, area);
                checkbox->setChecked(filter.systems.contains(item.slug));
                connect(checkbox, &QCheckBox::toggled, this, [this, item, area](bool shouldInclude)
                {
                    if (shouldInclude)
                    {
                        filter.systems.append(item.slug);
                    }
                    else
                    {
                        filter.systems.removeOne(item.slug);
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
            layout = new QVBoxLayout(area);
            layout->setAlignment(Qt::AlignTop);
            foreach (api::MetaTag item, this->data.filters.features)
            {
                auto filterCheckbox = new FilterCheckbox(item.name, area);
                filterCheckbox->setInclude(filter.features.contains(item.slug));
                filterCheckbox->setExclude(filter.excludeFeatures.contains(item.slug));
                connect(filterCheckbox, &FilterCheckbox::include, this, [this, item, area](bool shouldInclude)
                {
                    if (shouldInclude)
                    {
                        filter.features.append(item.slug);
                    }
                    else
                    {
                        filter.features.removeOne(item.slug);
                    }
                    page = 1;
                    area->setChangedFilters(filter.features.count() + filter.excludeFeatures.count());
                    fetchData();
                });
                connect(filterCheckbox, &FilterCheckbox::exclude, this, [this, item, area](bool shouldExclude)
                {
                    if (shouldExclude)
                    {
                        filter.excludeFeatures.append(item.slug);
                    }
                    else
                    {
                        filter.excludeFeatures.removeOne(item.slug);
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
            layout = new QVBoxLayout(area);
            layout->setAlignment(Qt::AlignTop);
            foreach (api::MetaTag item, this->data.filters.languages)
            {
                auto checkbox = new QCheckBox(item.name, area);
                checkbox->setChecked(filter.languages.contains(item.slug));
                connect(checkbox, &QCheckBox::toggled, this, [this, item, area](bool shouldInclude)
                {
                    if (shouldInclude)
                    {
                        filter.languages.append(item.slug);
                    }
                    else
                    {
                        filter.languages.removeOne(item.slug);
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
                ui->contentsStack->setCurrentWidget(ui->emptyPage);
            }
            else
            {
                paginator->changePages(page, this->data.pages);
                layoutResults();
            }

            networkReply->deleteLater();
        }
    });
    connect(networkReply, &QNetworkReply::errorOccurred, this, [=](QNetworkReply::NetworkError error)
    {
        if (error != QNetworkReply::NoError)
        {
            qDebug() << error;
            networkReply->deleteLater();
        }
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
        layoutResults();
    }
}


void AllGamesPage::on_listModeButton_clicked()
{
    if (gridLayout)
    {
        gridLayout = false;
        layoutResults();
    }
}

