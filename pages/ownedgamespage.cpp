#include "ownedgamespage.h"
#include "ui_ownedgamespage.h"

#include <QJsonDocument>
#include <QMenu>
#include <QNetworkReply>

#include "../api/utils/ownedproductserialization.h"
#include "../layouts/flowlayout.h"
#include "../widgets/ownedproductgriditem.h"

OwnedGamesPage::OwnedGamesPage(QWidget *parent) :
    BasePage(parent),
    ui(new Ui::OwnedGamesPage)
{
    ui->setupUi(this);

    auto settingsMenu = new QMenu(ui->settingsToolButton);

    auto gridModeItem = settingsMenu->addAction("Grid");
    gridModeItem->setCheckable(true);
    gridModeItem->setChecked(true);
    auto listModeItem = settingsMenu->addAction("List");
    listModeItem->setCheckable(true);

    settingsMenu->addSeparator();
    auto settingsSubmenu = settingsMenu->addMenu("Customize grid view");
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
        settingsSubmenu->setVisible(toggled);
    });

    settingsSubmenu = settingsMenu->addMenu("Customize list view");
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
    settingsSubmenu->setVisible(false);

    connect(listModeItem, &QAction::toggled, this, [this, settingsSubmenu, gridModeItem](bool toggled)
    {
        if (toggled)
        {

        }
        gridModeItem->setChecked(!toggled);
        settingsSubmenu->setVisible(toggled);
    });

    settingsMenu->addSeparator();

    settingsSubmenu = settingsMenu->addMenu("Sort by");
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

    settingsSubmenu = settingsMenu->addMenu("Group by");
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

    ui->settingsToolButton->setMenu(settingsMenu);

    ui->contentsStack->setCurrentWidget(ui->loaderPage);
    ui->resultsStack->setCurrentWidget(ui->resultsGridPage);
    ui->resultsGridScrollAreaContents->setLayout(new FlowLayout(ui->resultsGridScrollAreaContents));
}

OwnedGamesPage::~OwnedGamesPage()
{
    delete ui;
}

void OwnedGamesPage::setApiClient(api::GogApiClient *apiClient)
{
    this->apiClient = apiClient;
}

void OwnedGamesPage::updateData()
{
    clear();
    auto networkReply = apiClient->getOwnedProducts(searchQuery);
    connect(networkReply, &QNetworkReply::finished, this, [=]()
    {
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
                api::OwnedProduct item;
                foreach (item, data.products)
                {
                    ui->resultsGridScrollAreaContents->layout()->addWidget(new OwnedProductGridItem(item, apiClient, ui->resultsGridPage));
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

void OwnedGamesPage::clear()
{
    ui->contentsStack->setCurrentWidget(ui->loaderPage);
    while (!ui->resultsGridScrollAreaContents->layout()->isEmpty())
    {
        auto item = ui->resultsGridScrollAreaContents->layout()->itemAt(0);
        ui->resultsGridScrollAreaContents->layout()->removeItem(item);
        item->widget()->deleteLater();
        delete item;
    }
}

void OwnedGamesPage::initialize()
{
    updateData();
}

void OwnedGamesPage::on_searchLineEdit_textChanged(const QString &arg1)
{
    searchQuery = arg1;
    updateData();
}

