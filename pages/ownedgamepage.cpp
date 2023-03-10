#include "ownedgamepage.h"
#include "ui_ownedgamepage.h"

#include <QJsonDocument>
#include <QNetworkReply>
#include <QScrollBar>

#include "../api/models/ownedproductinfo.h"
#include "../api/utils/ownedproductinfoserialization.h"

OwnedGamePage::OwnedGamePage(QWidget *parent) :
    BasePage(parent),
    ui(new Ui::OwnedGamePage)
{
    ui->setupUi(this);
    ui->contentsStack->setCurrentWidget(ui->loaderPage);
    ui->resultsPageTabWidget->setCurrentWidget(ui->overviewTab);
    ui->extrasTabLayout->setAlignment(Qt::AlignTop);
}

OwnedGamePage::~OwnedGamePage()
{
    delete ui;
}

void OwnedGamePage::setApiClient(api::GogApiClient *apiClient)
{
    this->apiClient = apiClient;
}

void OwnedGamePage::clear()
{
    ui->contentsStack->setCurrentWidget(ui->loaderPage);
    ui->resultsPageTabWidget->setCurrentWidget(ui->overviewTab);
    ui->descriptionLabel->clear();
    ui->changelogTextBrowser->clear();
    ui->overviewTabScrollArea->verticalScrollBar()->setValue(0);
    while (!ui->extrasTabLayout->isEmpty())
    {
        auto item = ui->extrasTabLayout->itemAt(0);
        ui->extrasTabLayout->removeItem(item);
        item->widget()->deleteLater();
        delete item;
    }
}

void OwnedGamePage::initialize(const QVariant &data)
{
    auto networkReply = apiClient->getOwnedProductInfo(data.toLongLong(), "en-US");
    connect(networkReply, &QNetworkReply::finished, this, [=](){
        if (networkReply->error() == QNetworkReply::NoError)
        {
            auto resultJson = QJsonDocument::fromJson(QString(networkReply->readAll()).toUtf8()).object();
            api::GetOwnedProductInfoResponse data;
            parseGetOwnedProductInfoResponse(resultJson, data);

            ui->titleLabel->setText(data.mainProductInfo.title);
            ui->descriptionLabel->setText(data.descriptionFull);

            ui->resultsPageTabWidget->setTabVisible(ui->resultsPageTabWidget->indexOf(ui->changelogTab), !data.changelog.isEmpty());
            ui->changelogTextBrowser->setHtml(data.changelog);

            bool showDownloads = false;
            if (!data.mainProductInfo.downloads.bonusContent.isEmpty())
            {
                ui->extrasTabLayout->addWidget(new QLabel("Goodies", ui->extrasTab));
                foreach (api::Download item, data.mainProductInfo.downloads.bonusContent)
                {
                    ui->extrasTabLayout->addWidget(new QLabel(item.name, ui->extrasTab));
                }
                showDownloads = true;
            }
            if (!data.mainProductInfo.downloads.installers.isEmpty())
            {
                ui->extrasTabLayout->addWidget(new QLabel("Backup offline installers", ui->extrasTab));
                foreach (api::Download item, data.mainProductInfo.downloads.installers)
                {
                    ui->extrasTabLayout->addWidget(new QLabel(item.name, ui->extrasTab));
                }
                showDownloads = true;
            }
            if (!data.mainProductInfo.downloads.patches.isEmpty())
            {
                ui->extrasTabLayout->addWidget(new QLabel("Patches", ui->extrasTab));
                foreach (api::Download item, data.mainProductInfo.downloads.installers)
                {
                    ui->extrasTabLayout->addWidget(new QLabel(item.name, ui->extrasTab));
                }
                showDownloads = true;
            }
            if (!data.mainProductInfo.downloads.languagePacks.isEmpty())
            {
                showDownloads = true;
            }
            ui->resultsPageTabWidget->setTabVisible(ui->resultsPageTabWidget->indexOf(ui->extrasTab), showDownloads);

            ui->contentsStack->setCurrentWidget(ui->resultsPage);

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
