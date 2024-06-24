#include "ownedgamepage.h"
#include "ui_ownedgamepage.h"

#include <QJsonDocument>
#include <QJsonObject>
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
    auto systemLocale = QLocale::system();
    auto networkReply = apiClient->getOwnedProductInfo(data.toString(),
                                                       QLocale::languageToCode(systemLocale.language(), QLocale::ISO639Part1));
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
            QVector<const api::BonusDownload *> goodies;
            for (const api::BonusDownload &bonusDownload : std::as_const(data.mainProductInfo.downloads.bonusContent))
            {
                goodies << &bonusDownload;
            }
            QVector<const api::GameDownload *> installers;
            for (const api::GameDownload &installerDownload : std::as_const(data.mainProductInfo.downloads.installers))
            {
                installers << &installerDownload;
            }
            QVector<const api::GameDownload *> patches;
            for (const api::GameDownload &patchDownload : std::as_const(data.mainProductInfo.downloads.patches))
            {
                patches << &patchDownload;
            }
            QVector<const api::GameDownload *> languagePacks;
            for (const api::GameDownload &languagePackDownload : std::as_const(data.mainProductInfo.downloads.languagePacks))
            {
                languagePacks << &languagePackDownload;
            }
            for (const api::ProductInfo &dlc : std::as_const(data.expandedDlcs))
            {
                for (const api::BonusDownload &bonusDownload : std::as_const(dlc.downloads.bonusContent))
                {
                    goodies << &bonusDownload;
                }
                for (const api::GameDownload &installerDownload: std::as_const(dlc.downloads.installers))
                {
                    installers << &installerDownload;
                }
                for (const api::GameDownload &patchDownload : std::as_const(dlc.downloads.patches))
                {
                    patches << &patchDownload;
                }
                for (const api::GameDownload &languagePackDownload : std::as_const(dlc.downloads.languagePacks))
                {
                    languagePacks << &languagePackDownload;
                }
            }

            if (!goodies.isEmpty())
            {
                ui->extrasTabScrollAreaContentsLayout->addWidget(new QLabel("Goodies", ui->extrasTab));
                for (const api::Download *item : std::as_const(goodies))
                {
                    ui->extrasTabScrollAreaContentsLayout->addWidget(new QLabel(item->name, ui->extrasTab));
                }
                showDownloads = true;
            }
            if (!installers.isEmpty())
            {
                ui->extrasTabScrollAreaContentsLayout->addWidget(new QLabel("Offline backup installers", ui->extrasTab));
                for (const api::Download *item : std::as_const(installers))
                {
                    ui->extrasTabScrollAreaContentsLayout->addWidget(new QLabel(item->name, ui->extrasTab));
                }
                for (const api::Download *item : std::as_const(languagePacks))
                {
                    ui->extrasTabScrollAreaContentsLayout->addWidget(new QLabel(item->name, ui->extrasTab));
                }
                for (const api::Download *item : std::as_const(patches))
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

