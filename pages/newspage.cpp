#include "newspage.h"
#include "ui_newspage.h"

 #include <QDesktopServices>
#include <QJsonDocument>
#include <QNetworkReply>

#include "../api/utils/newsserialization.h"
#include "../widgets/clickablelabel.h"

NewsPage::NewsPage(QWidget *parent) :
    BasePage(parent),
    bannerReply(nullptr),
    newsHeadlinesReply(nullptr),
    newsPageReply(nullptr),
    ui(new Ui::NewsPage)
{
    ui->setupUi(this);

    connect(ui->descriptionWebEngineView->page(), &QWebEnginePage::contentsSizeChanged,
            this, [this](const QSizeF &size)
    {
        ui->descriptionWebEngineView->setFixedHeight(size.height());
    });
}

NewsPage::~NewsPage()
{
    if (bannerReply != nullptr)
    {
        bannerReply->abort();
    }
    if (newsHeadlinesReply != nullptr)
    {
        newsHeadlinesReply->abort();
    }
    if (newsPageReply != nullptr)
    {
        newsPageReply->abort();
    }
    delete ui;
}

void NewsPage::setApiClient(api::GogApiClient *apiClient)
{
    this->apiClient = apiClient;
}

void NewsPage::initialize(const QVariant &data)
{
    ui->contentStackedWidget->setCurrentWidget(ui->contentLoadingPage);

    QString systemLanguage = QLocale::languageToCode(QLocale::system().language(), QLocale::ISO639Part1);
    newsHeadlinesReply = apiClient->getNews(0, systemLanguage, 6);
    connect(newsHeadlinesReply, &QNetworkReply::finished,
            this, [this]()
    {
        auto networkReply = newsHeadlinesReply;
        newsHeadlinesReply = nullptr;

        if (networkReply->error() == QNetworkReply::NoError)
        {
            auto resultJson = QJsonDocument::fromJson(QString(networkReply->readAll()).toUtf8()).object();
            api::GetNewsResponse data;
            parseNewsResponse(resultJson, data);

            auto systemLocale = QLocale::system();
            for (const api::NewsItem &item : std::as_const(data.items))
            {
                auto headlineItem = new QWidget(ui->headlinesResultsPage);
                auto headlineItemLayout = new QVBoxLayout;
                headlineItemLayout->setContentsMargins(0, 0, 0, 0);
                headlineItemLayout->setSpacing(4);
                headlineItem->setLayout(headlineItemLayout);

                auto headlineItemTitleLabel = new ClickableLabel(headlineItem);
                headlineItemTitleLabel->setCursor(Qt::PointingHandCursor);
                connect(headlineItemTitleLabel, &ClickableLabel::clicked,
                        this, [this]()
                {
                    // TODO: navigate to clicked item
                });
                headlineItemTitleLabel->setText(QString("%1 (%2)").arg(item.title).arg(item.commentsCount));
                headlineItemTitleLabel->setWordWrap(true);
                headlineItemLayout->addWidget(headlineItemTitleLabel);

                auto headlineItemPublishedLabel = new QLabel(headlineItem);
                headlineItemPublishedLabel->setText(systemLocale.toString(item.publishDate, QLocale::NarrowFormat));
                headlineItemLayout->addWidget(headlineItemPublishedLabel);

                ui->headlinesResultsPageLayout->addWidget(headlineItem);
            }
            ui->headlinesResultsPageLayout->addStretch();
            ui->headlinesStackedWidget->setCurrentWidget(ui->headlinesResultsPage);
        }
        else if (networkReply->error() != QNetworkReply::OperationCanceledError)
        {
            qDebug() << networkReply->error()
                     << networkReply->errorString()
                     << QString(networkReply->readAll()).toUtf8();
        }
        networkReply->deleteLater();
    });
    newsPageReply = apiClient->getNews(0, systemLanguage, 11);
    connect(newsPageReply, &QNetworkReply::finished,
            this, [this]()
    {
        auto networkReply = newsPageReply;
        newsPageReply = nullptr;

        if (networkReply->error() == QNetworkReply::NoError)
        {
            auto resultJson = QJsonDocument::fromJson(QString(networkReply->readAll()).toUtf8()).object();
            api::GetNewsResponse data;
            parseNewsResponse(resultJson, data);

            api::NewsItem displayedNews = data.items[0];
            ui->titleLabel->setText(displayedNews.title);
            bannerReply = apiClient->getAnything(displayedNews.imageLarge);
            connect(bannerReply, &QNetworkReply::finished,
                    this, [this]()
            {
                auto networkReply = bannerReply;
                bannerReply = nullptr;

                if (networkReply->error() == QNetworkReply::NoError)
                {
                    QPixmap bannerImage;
                    bannerImage.loadFromData(networkReply->readAll());

                    QSize imageSize = bannerImage.size() / 2;
                    ui->coverLabel->setMinimumSize(imageSize);
                    ui->coverLabel->setMaximumSize(imageSize);
                    ui->coverLabel->setPixmap(bannerImage.scaled(imageSize, Qt::KeepAspectRatioByExpanding));
                }
                else if (networkReply->error() != QNetworkReply::OperationCanceledError)
                {
                    qDebug() << networkReply->error()
                             << networkReply->errorString()
                             << QString(networkReply->readAll()).toUtf8();
                }
                networkReply->deleteLater();
            });

            ui->publishedLabel->setText(QLocale::system().toString(displayedNews.publishDate, QLocale::ShortFormat));
            ui->descriptionWebEngineView->setHtml(displayedNews.body);
            ui->seeCommentsButton->setText(ui->seeCommentsButton->text().arg(displayedNews.commentsCount));

            QUrl forumLink(displayedNews.forumThreadLink);
            connect(ui->seeCommentsButton, &QPushButton::clicked,
                    this, [this, forumLink]()
            {
                QDesktopServices::openUrl(forumLink);
            });
            ui->goBackButton->setEnabled(false);
            ui->goForwardButton->setEnabled(false);
            ui->contentStackedWidget->setCurrentWidget(ui->contentResultsPage);
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

void NewsPage::switchUiAuthenticatedState(bool authenticated)
{

}
