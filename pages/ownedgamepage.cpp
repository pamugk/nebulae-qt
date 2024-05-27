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
    ui->extrasTabScrollAreaContentsLayout->setAlignment(Qt::AlignTop);
}

OwnedGamePage::~OwnedGamePage()
{
    delete ui;
}

void OwnedGamePage::setApiClient(api::GogApiClient *apiClient)
{
    this->apiClient = apiClient;
}

void OwnedGamePage::initialize(const QVariant &data)
{
    auto networkReply = apiClient->getOwnedProductInfo(data.toLongLong(), "en-US");
    connect(networkReply, &QNetworkReply::finished, this, [=](){
        if (networkReply->error() == QNetworkReply::NoError)
        {
            auto locale = QLocale::system();
            auto resultJson = QJsonDocument::fromJson(QString(networkReply->readAll()).toUtf8()).object();
            api::GetOwnedProductInfoResponse data;
            parseGetOwnedProductInfoResponse(resultJson, data);

            ui->titleLabel->setText(data.mainProductInfo.title);
            ui->descriptionLabel->setText(data.descriptionLead);
            ui->descriptionLabel->setProperty("fullText", data.descriptionFull);
            ui->expandDescriptionButton->setVisible(true);
            ui->genresLabel->setVisible(false);
            ui->styleLabel->setVisible(false);
            if (data.mainProductInfo.releaseDate.isNull())
            {
                ui->releaseDateLabel->setVisible(false);
            }
            else
            {
                ui->releaseDateLabel->setText("<b>Release date:</b> " + locale.toString(data.mainProductInfo.releaseDate.date(), QLocale::ShortFormat));
                ui->releaseDateLabel->setVisible(true);
            }
            ui->developerLabel->setVisible(false);

            ui->resultsPageTabWidget->setTabVisible(ui->resultsPageTabWidget->indexOf(ui->changelogTab), !data.changelog.isEmpty());
            ui->changelogTextBrowser->setHtml(data.changelog);

            bool showDownloads = false;
            QVector<api::BonusDownload*> goodies;
            for(int i = 0; i < data.mainProductInfo.downloads.bonusContent.count(); i++)
            {
                goodies.append(&data.mainProductInfo.downloads.bonusContent[i]);
            }
            QVector<api::GameDownload*> installers;
            for(int i = 0; i < data.mainProductInfo.downloads.installers.count(); i++)
            {
                installers.append(&data.mainProductInfo.downloads.installers[i]);
            }
            QVector<api::GameDownload*> patches;
            for(int i = 0; i < data.mainProductInfo.downloads.patches.count(); i++)
            {
                patches.append(&data.mainProductInfo.downloads.patches[i]);
            }
            QVector<api::GameDownload*> languagePacks;
            for(int i = 0; i < data.mainProductInfo.downloads.languagePacks.count(); i++)
            {
                languagePacks.append(&data.mainProductInfo.downloads.languagePacks[i]);
            }
            for (int i = 0; i < data.expandedDlcs.count(); i++)
            {
                for(int j = 0; j < data.expandedDlcs[i].downloads.bonusContent.count(); j++)
                {
                    goodies.append(&data.expandedDlcs[i].downloads.bonusContent[j]);
                }
                for(int j = 0; j < data.expandedDlcs[i].downloads.installers.count(); j++)
                {
                    installers.append(&data.expandedDlcs[i].downloads.installers[j]);
                }
                for(int j = 0; j < data.expandedDlcs[i].downloads.patches.count(); j++)
                {
                    patches.append(&data.expandedDlcs[i].downloads.patches[j]);
                }
                for(int j = 0; j < data.expandedDlcs[i].downloads.languagePacks.count(); j++)
                {
                    languagePacks.append(&data.expandedDlcs[i].downloads.languagePacks[j]);
                }
            }

            if (!goodies.isEmpty())
            {
                ui->extrasTabScrollAreaContentsLayout->addWidget(new QLabel("Goodies", ui->extrasTab));
                foreach (api::Download *item, goodies)
                {
                    ui->extrasTabScrollAreaContentsLayout->addWidget(new QLabel(item->name, ui->extrasTab));
                }
                showDownloads = true;
            }
            if (!installers.isEmpty())
            {
                ui->extrasTabScrollAreaContentsLayout->addWidget(new QLabel("Offline backup installers", ui->extrasTab));
                foreach (api::Download *item, installers)
                {
                    ui->extrasTabScrollAreaContentsLayout->addWidget(new QLabel(item->name, ui->extrasTab));
                }
                foreach (api::Download *item, languagePacks)
                {
                    ui->extrasTabScrollAreaContentsLayout->addWidget(new QLabel(item->name, ui->extrasTab));
                }
                foreach (api::Download *item, patches)
                {
                    ui->extrasTabScrollAreaContentsLayout->addWidget(new QLabel(item->name, ui->extrasTab));
                }
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

void OwnedGamePage::switchUiAuthenticatedState(bool authenticated)
{

}

void OwnedGamePage::on_expandDescriptionButton_clicked()
{
    ui->descriptionLabel->setText(ui->descriptionLabel->property("fullText").toString());
    ui->expandDescriptionButton->setVisible(false);
}

