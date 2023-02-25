#include "allgamespage.h"
#include "ui_allgamespage.h"

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
}

AllGamesPage::~AllGamesPage()
{
    delete ui;
}

void AllGamesPage::setApiClient(GogApiClient *apiClient)
{
    this->apiClient = apiClient;
}

void AllGamesPage::clear()
{
    ui->contentsStack->setCurrentWidget(ui->loaderPage);
    while (!ui->filtersScrollAreaLayout->isEmpty())
    {
        ui->filtersScrollAreaLayout->removeItem(ui->filtersScrollAreaLayout->itemAt(0));
    }
    while (!ui->resultsScrollAreaLayout->isEmpty())
    {
        ui->resultsScrollAreaLayout->removeItem(ui->resultsScrollAreaLayout->itemAt(0));
    }
}

void AllGamesPage::initialize()
{
    auto networkReply = apiClient->searchCatalog("bestselling", false, "RU", "en-US", "RUB");
    ui->contentsStack->setCurrentWidget(ui->loaderPage);
    connect(networkReply, &QNetworkReply::finished, this, [=](){
        if (networkReply->error() == QNetworkReply::NoError)
        {
            auto resultJson = QJsonDocument::fromJson(QString(networkReply->readAll()).toUtf8()).object();
            SearchCatalogResponse data;
            parseSearchCatalogResponse(resultJson, data);

            auto priceArea = new CollapsibleArea("Price range", ui->filtersScrollAreaContents);
            auto priceAreaLayout = new QVBoxLayout(priceArea);
            priceAreaLayout->addWidget(new QCheckBox("Show only free games", priceArea));
            priceArea->setContentLayout(priceAreaLayout);

            auto releaseStatusArea = new CollapsibleArea("Release Status", ui->filtersScrollAreaContents);
            auto releaseStatusAreaLayout = new QVBoxLayout(releaseStatusArea);
            releaseStatusArea->setContentLayout(releaseStatusAreaLayout);

            MetaTag item;

            auto genresArea = new CollapsibleArea("Genres", ui->filtersScrollAreaContents);
            auto genresAreaLayout = new QVBoxLayout(genresArea);
            foreach (item, data.filters.genres)
            {
                genresAreaLayout->addWidget(new FilterCheckbox(item.name, genresArea));
            }
            genresArea->setContentLayout(genresAreaLayout);

            auto tagsArea = new CollapsibleArea("Tags", ui->filtersScrollAreaContents);
            auto tagsAreaLayout = new QVBoxLayout(tagsArea);
            foreach (item, data.filters.tags)
            {
                tagsAreaLayout->addWidget(new FilterCheckbox(item.name, tagsArea));
            }
            tagsArea->setContentLayout(tagsAreaLayout);

            auto osArea = new CollapsibleArea("Operating Systems", ui->filtersScrollAreaContents);
            auto osAreaLayout = new QVBoxLayout(osArea);
            foreach (item, data.filters.systems)
            {
                osAreaLayout->addWidget(new FilterCheckbox(item.name, osArea));
            }
            osArea->setContentLayout(osAreaLayout);

            auto featuresArea = new CollapsibleArea("Features", ui->filtersScrollAreaContents);
            auto featuresAreaLayout = new QVBoxLayout(featuresArea);
            foreach (item, data.filters.features)
            {
                featuresAreaLayout->addWidget(new FilterCheckbox(item.name, featuresArea));
            }
            featuresArea->setContentLayout(featuresAreaLayout);

            auto releaseDateArea = new CollapsibleArea("Release Date", ui->filtersScrollAreaContents);

            auto languagesArea = new CollapsibleArea("Languages", ui->filtersScrollAreaContents);
            auto languagesAreaLayout = new QVBoxLayout(languagesArea);
            foreach (item, data.filters.languages)
            {
                languagesAreaLayout->addWidget(new FilterCheckbox(item.name, languagesArea));
            }
            languagesArea->setContentLayout(languagesAreaLayout);

            ui->filtersScrollAreaLayout->addWidget(priceArea);
            ui->filtersScrollAreaLayout->addWidget(releaseStatusArea);
            ui->filtersScrollAreaLayout->addWidget(genresArea);
            ui->filtersScrollAreaLayout->addWidget(tagsArea);
            ui->filtersScrollAreaLayout->addWidget(osArea);
            ui->filtersScrollAreaLayout->addWidget(featuresArea);
            ui->filtersScrollAreaLayout->addWidget(releaseDateArea);
            ui->filtersScrollAreaLayout->addWidget(languagesArea);

            ui->totalLabel->setText(QString("Showing %1 games").arg(QString::number(data.productCount)));
            ui->pagesLabel->setText(QString("%1 of %2").arg(QString::number(1), QString::number(data.pages)));

            if (data.products.isEmpty())
            {
                ui->contentsStack->setCurrentWidget(ui->nothingFoundPage);
            }
            else
            {
                CatalogProduct product;
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
