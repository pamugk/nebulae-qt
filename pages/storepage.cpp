#include "storepage.h"
#include "ui_storepage.h"

#include <QJsonDocument>
#include <QNetworkReply>

#include "../api/utils/newsserialization.h"
#include "../widgets/newsitemtile.h"

StorePage::StorePage(QWidget *parent) :
    BasePage(parent),
    apiClient(nullptr),
    newsReply(nullptr),
    ui(new Ui::StorePage)
{
    ui->setupUi(this);
}

StorePage::~StorePage()
{
    if (newsReply != nullptr)
    {
        newsReply->abort();
    }
    delete ui;
}

void StorePage::setApiClient(api::GogApiClient *apiClient)
{
    this->apiClient = apiClient;
}

void StorePage::getNews()
{
    ui->newsStackedWidget->setCurrentWidget(ui->newsLoadingPage);

    QString systemLanguage = QLocale::languageToCode(QLocale::system().language(), QLocale::ISO639Part1);
    newsReply = apiClient->getNews(0, systemLanguage, 11);
    connect(newsReply, &QNetworkReply::finished, this, [this]()
    {
        auto networkReply = newsReply;
        newsReply = nullptr;

        if (networkReply->error() == QNetworkReply::NoError)
        {
            auto resultJson = QJsonDocument::fromJson(QString(networkReply->readAll()).toUtf8()).object();
            api::GetNewsResponse data;
            parseNewsResponse(resultJson, data);

            bool primary = true;
            for (const api::NewsItem &item : std::as_const(data.items))
            {
                auto itemTile = new NewsItemTile(item, primary, apiClient, ui->newsScrollAreaContents);
                primary = false;
                connect(itemTile, &NewsItemTile::navigateToNewsItem, this,
                        [this](unsigned long long newsId){

                });
                ui->newsScrollAreaContentsLayout->addWidget(itemTile);
            }
            ui->newsStackedWidget->setCurrentWidget(ui->newsResultsPage);
        }
        else if (networkReply->error() != QNetworkReply::OperationCanceledError)
        {
            qDebug() << networkReply->error()
                     << networkReply->errorString()
                     << QString(networkReply->readAll()).toUtf8();
        }
        networkReply->deleteLater();
    });
}

void StorePage::initialize(const QVariant &data)
{
    getNews();
}

void StorePage::switchUiAuthenticatedState(bool authenticated)
{

}

