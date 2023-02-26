#include "allgamespage.h"
#include "ui_allgamespage.h"

#include <algorithm>

#include <QCheckBox>
#include <QJsonDocument>
#include <QNetworkReply>

#include "../api/models/catalog.h"
#include "../api/models/metatag.h"
#include "../api/utils/catalogserialization.h"
#include "../widgets/collapsiblearea.h"
#include "../widgets/filtercheckbox.h"
#include "../widgets/storegridtile.h"
#include "../widgets/storelistitem.h"

AllGamesPage::AllGamesPage(QWidget *parent) :
    BasePage(parent),
    ui(new Ui::AllGamesPage)
{
    ui->setupUi(this);
    ui->filtersScrollAreaLayout->setAlignment(Qt::AlignTop);
    ui->resultsScrollAreaLayout->setAlignment(Qt::AlignTop);

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
}

AllGamesPage::~AllGamesPage()
{
    delete ui;
}

void AllGamesPage::setApiClient(api::GogApiClient *apiClient)
{
    this->apiClient = apiClient;
}

void AllGamesPage::fetchData()
{
    ui->contentsStack->setCurrentWidget(ui->loaderPage);
    while (!ui->resultsScrollAreaLayout->isEmpty())
    {
        auto item = ui->resultsScrollAreaLayout->itemAt(0);
        ui->resultsScrollAreaLayout->removeItem(item);
        item->widget()->deleteLater();
        delete item;
    }
    auto networkReply = apiClient->searchCatalog(orders[currentSortOrder], filter, "RU", "en-US", "RUB");
    connect(networkReply, &QNetworkReply::finished, this, [=](){
        if (networkReply->error() == QNetworkReply::NoError)
        {
            auto resultJson = QJsonDocument::fromJson(QString(networkReply->readAll()).toUtf8()).object();
            api::SearchCatalogResponse data;
            parseSearchCatalogResponse(resultJson, data);

            ui->totalLabel->setText(QString("Showing %1 games").arg(QString::number(data.productCount)));
            ui->pagesLabel->setText(QString("%1 of %2").arg(QString::number(1), QString::number(data.pages)));

            if (data.products.isEmpty())
            {
                ui->contentsStack->setCurrentWidget(ui->emptyPage);
            }
            else
            {
                api::CatalogProduct product;
                foreach (product, data.products)
                {
                    ui->resultsScrollAreaLayout->addWidget(new StoreListItem(product, apiClient, ui->resultsScrollAreaContents));
                }
                ui->contentsStack->setCurrentWidget(ui->resultsPage);
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

void AllGamesPage::clear()
{
    ui->contentsStack->setCurrentWidget(ui->loaderPage);
    while (!ui->filtersScrollAreaLayout->isEmpty())
    {
        auto item = ui->filtersScrollAreaLayout->itemAt(0);
        ui->filtersScrollAreaLayout->removeItem(item);
        item->widget()->deleteLater();
        delete item;
    }
    while (!ui->resultsScrollAreaLayout->isEmpty())
    {
        auto item = ui->resultsScrollAreaLayout->itemAt(0);
        ui->resultsScrollAreaLayout->removeItem(item);
        item->widget()->deleteLater();
        delete item;
    }
}

void AllGamesPage::initialize()
{
    auto networkReply = apiClient->searchCatalog(orders[currentSortOrder], filter, "RU", "en-US", "RUB");
    ui->filtersScrollArea->setVisible(false);
    ui->contentsStack->setCurrentWidget(ui->loaderPage);
    connect(networkReply, &QNetworkReply::finished, this, [=](){
        if (networkReply->error() == QNetworkReply::NoError)
        {
            auto resultJson = QJsonDocument::fromJson(QString(networkReply->readAll()).toUtf8()).object();
            api::SearchCatalogResponse data;
            parseSearchCatalogResponse(resultJson, data);

            int maxWidth = 0;
            CollapsibleArea *area;
            QLayout *layout;
            QCheckBox *checkbox;

            checkbox = new QCheckBox("Show only discounted", ui->filtersScrollAreaContents);
            maxWidth = std::max(maxWidth, checkbox->width());
            connect(checkbox, &QCheckBox::toggled, this, [this](bool value)
            {
                filter.discounted = value;
                fetchData();
            });
            ui->filtersScrollAreaLayout->addWidget(checkbox);

            checkbox = new QCheckBox("Hide DLCs and extras", ui->filtersScrollAreaContents);
            maxWidth = std::max(maxWidth, checkbox->width());
            connect(checkbox, &QCheckBox::toggled, this, [this](bool value)
            {
                filter.productTypes = value ? QStringList({"game","pack"}) : QStringList({"game","pack","dlc","extras"});
                fetchData();
            });
            ui->filtersScrollAreaLayout->addWidget(checkbox);

            checkbox = new QCheckBox("Hide all owned products", ui->filtersScrollAreaContents);
            connect(checkbox, &QCheckBox::toggled, this, [this](bool value)
            {
                filter.hideOwned = value;
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

            api::MetaTag item;
            area = new CollapsibleArea("Release Status", ui->filtersScrollAreaContents);
            layout = new QVBoxLayout(area);
            layout->setAlignment(Qt::AlignTop);
            foreach (item, data.filters.releaseStatuses)
            {
                auto filterCheckbox = new FilterCheckbox(item.name, area);
                connect(filterCheckbox, &FilterCheckbox::include, this, [this, item](bool shouldInclude)
                {
                    if (shouldInclude)
                    {
                        filter.releaseStatuses.append(item.slug);
                    }
                    else
                    {
                        filter.releaseStatuses.removeOne(item.slug);
                    }
                    fetchData();
                });
                connect(filterCheckbox, &FilterCheckbox::exclude, this, [this, item](bool shouldExclude)
                {
                    if (shouldExclude)
                    {
                        filter.excludeReleaseStatuses.append(item.slug);
                    }
                    else
                    {
                        filter.excludeReleaseStatuses.removeOne(item.slug);
                    }
                    fetchData();
                });
                layout->addWidget(filterCheckbox);
            }
            area->setContentLayout(layout);
            maxWidth = std::max(maxWidth, layout->sizeHint().width());
            ui->filtersScrollAreaLayout->addWidget(area);

            area = new CollapsibleArea("Genres", ui->filtersScrollAreaContents);
            layout = new QVBoxLayout(area);
            layout->setAlignment(Qt::AlignTop);
            foreach (item, data.filters.genres)
            {
                auto filterCheckbox = new FilterCheckbox(item.name, area);
                connect(filterCheckbox, &FilterCheckbox::include, this, [this, item](bool shouldInclude)
                {
                    if (shouldInclude)
                    {
                        filter.genres.append(item.slug);
                    }
                    else
                    {
                        filter.genres.removeOne(item.slug);
                    }
                    fetchData();
                });
                connect(filterCheckbox, &FilterCheckbox::exclude, this, [this, item](bool shouldExclude)
                {
                    if (shouldExclude)
                    {
                        filter.excludeGenres.append(item.slug);
                    }
                    else
                    {
                        filter.excludeGenres.removeOne(item.slug);
                    }
                    fetchData();
                });
                layout->addWidget(filterCheckbox);
            }
            area->setContentLayout(layout);
            maxWidth = std::max(maxWidth, layout->sizeHint().width());
            ui->filtersScrollAreaLayout->addWidget(area);

            area = new CollapsibleArea("Tags", ui->filtersScrollAreaContents);
            layout = new QVBoxLayout(area);
            layout->setAlignment(Qt::AlignTop);
            foreach (item, data.filters.tags)
            {
                auto filterCheckbox = new FilterCheckbox(item.name, area);
                connect(filterCheckbox, &FilterCheckbox::include, this, [this, item](bool shouldInclude)
                {
                    if (shouldInclude)
                    {
                        filter.tags.append(item.slug);
                    }
                    else
                    {
                        filter.tags.removeOne(item.slug);
                    }
                    fetchData();
                });
                connect(filterCheckbox, &FilterCheckbox::exclude, this, [this, item](bool shouldExclude)
                {
                    if (shouldExclude)
                    {
                        filter.excludeTags.append(item.slug);
                    }
                    else
                    {
                        filter.excludeTags.removeOne(item.slug);
                    }
                    fetchData();
                });
                layout->addWidget(filterCheckbox);
            }
            layout->setAlignment(Qt::AlignTop);
            area->setContentLayout(layout);
            maxWidth = std::max(maxWidth, layout->sizeHint().width());
            ui->filtersScrollAreaLayout->addWidget(area);

            area = new CollapsibleArea("Operating Systems", ui->filtersScrollAreaContents);
            layout = new QVBoxLayout(area);
            layout->setAlignment(Qt::AlignTop);
            foreach (item, data.filters.systems)
            {
                auto checkbox = new QCheckBox(item.name, area);
                connect(checkbox, &QCheckBox::toggled, this, [this, item](bool shouldInclude)
                {
                    if (shouldInclude)
                    {
                        filter.systems.append(item.slug);
                    }
                    else
                    {
                        filter.systems.removeOne(item.slug);
                    }
                    fetchData();
                });
                layout->addWidget(checkbox);
            }
            area->setContentLayout(layout);
            maxWidth = std::max(maxWidth, layout->sizeHint().width());
            ui->filtersScrollAreaLayout->addWidget(area);

            area = new CollapsibleArea("Features", ui->filtersScrollAreaContents);
            layout = new QVBoxLayout(area);
            layout->setAlignment(Qt::AlignTop);
            foreach (item, data.filters.features)
            {
                auto filterCheckbox = new FilterCheckbox(item.name, area);
                connect(filterCheckbox, &FilterCheckbox::include, this, [this, item](bool shouldInclude)
                {
                    if (shouldInclude)
                    {
                        filter.features.append(item.slug);
                    }
                    else
                    {
                        filter.features.removeOne(item.slug);
                    }
                    fetchData();
                });
                connect(filterCheckbox, &FilterCheckbox::exclude, this, [this, item](bool shouldExclude)
                {
                    if (shouldExclude)
                    {
                        filter.excludeFeatures.append(item.slug);
                    }
                    else
                    {
                        filter.excludeFeatures.removeOne(item.slug);
                    }
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
            layout = new QVBoxLayout(area);
            layout->setAlignment(Qt::AlignTop);
            foreach (item, data.filters.languages)
            {
                auto checkbox = new QCheckBox(item.name, area);
                connect(checkbox, &QCheckBox::toggled, this, [this, item](bool shouldInclude)
                {
                    if (shouldInclude)
                    {
                        filter.languages.append(item.slug);
                    }
                    else
                    {
                        filter.languages.removeOne(item.slug);
                    }
                    fetchData();
                });
                layout->addWidget(checkbox);
            }
            area->setContentLayout(layout);
            maxWidth = std::max(maxWidth, layout->sizeHint().width());
            ui->filtersScrollAreaLayout->addWidget(area);

            ui->filtersScrollArea->setFixedWidth(maxWidth + 20);
            ui->filtersScrollArea->setVisible(true);

            ui->totalLabel->setText(QString("Showing %1 games").arg(QString::number(data.productCount)));
            ui->pagesLabel->setText(QString("%1 of %2").arg(QString::number(1), QString::number(data.pages)));

            if (data.products.isEmpty())
            {
                ui->contentsStack->setCurrentWidget(ui->emptyPage);
            }
            else
            {
                api::CatalogProduct product;
                foreach (product, data.products)
                {
                    ui->resultsScrollAreaLayout->addWidget(new StoreListItem(product, apiClient, ui->resultsScrollAreaContents));
                }
                ui->contentsStack->setCurrentWidget(ui->resultsPage);
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

void AllGamesPage::on_lineEdit_textChanged(const QString &arg1)
{
    filter.query = arg1.trimmed();
    fetchData();
}


void AllGamesPage::on_sortOrderComboBox_currentIndexChanged(int index)
{
    if (index < 0 || index > 9)
    {
        index = 0;
    }
    currentSortOrder = index;
    fetchData();
}

