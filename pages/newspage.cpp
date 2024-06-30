#include "newspage.h"
#include "ui_newspage.h"

#include <QDesktopServices>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkReply>
#include <QStyle>

#include "../api/utils/newsserialization.h"
#include "../widgets/clickablelabel.h"

NewsPage::NewsPage(QWidget *parent) :
    StoreBasePage(Page::NEWS, parent),
    ui(new Ui::NewsPage)
{
    ui->setupUi(this);

    connect(ui->descriptionWebEngineView->page(), &QWebEnginePage::contentsSizeChanged,
            this, [this](const QSizeF &size)
    {
        ui->descriptionWebEngineView->setFixedHeight(size.height());
    });
    ui->descriptionWebEngineView->page()->setBackgroundColor(Qt::transparent);
}

NewsPage::~NewsPage()
{
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
    auto newsHeadlinesReply = apiClient->getNews(0, systemLanguage, 6);
    connect(newsHeadlinesReply, &QNetworkReply::finished,
            this, [this, networkReply = newsHeadlinesReply]()
    {
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
                        this, [this, id = item.id]()
                {
                    emit navigate({Page::NEWS, id});
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
    connect(this, &QObject::destroyed, newsHeadlinesReply, &QNetworkReply::abort);

    auto newsPageReply = apiClient->getNews(0, systemLanguage, 11);
    connect(newsPageReply, &QNetworkReply::finished,
            this, [this, id = data.toULongLong(), networkReply = newsPageReply]()
    {
        if (networkReply->error() == QNetworkReply::NoError)
        {
            auto resultJson = QJsonDocument::fromJson(QString(networkReply->readAll()).toUtf8()).object();
            api::GetNewsResponse data;
            parseNewsResponse(resultJson, data);

            for (std::size_t i = 0; i < data.items.count(); i++)
            {
                if (data.items[i].id == id)
                {
                    const api::NewsItem &displayedNews = data.items[i];
                    ui->titleLabel->setText(displayedNews.title);
                    auto bannerReply = apiClient->getAnything(displayedNews.imageLarge);
                    connect(bannerReply, &QNetworkReply::finished,
                            this, [this, networkReply = bannerReply]()
                    {
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
                    connect(this, &QObject::destroyed, bannerReply, &QNetworkReply::abort);

                    ui->publishedLabel->setText(QLocale::system().toString(displayedNews.publishDate, QLocale::ShortFormat));
                    ui->descriptionWebEngineView->setHtml(displayedNews.body);
                    ui->seeCommentsButton->setText(ui->seeCommentsButton->text().arg(displayedNews.commentsCount));

                    QUrl forumLink(displayedNews.forumThreadLink);
                    connect(ui->seeCommentsButton, &QPushButton::clicked,
                            this, [this, forumLink]()
                    {
                        QDesktopServices::openUrl(forumLink);
                    });

                    auto previousItemLabel = new ClickableLabel(ui->contentResultsScrollAreaContents);
                    previousItemLabel->setFixedHeight(58);
                    previousItemLabel->setWordWrap(true);
                    previousItemLabel->setAlignment(Qt::AlignTop | Qt::AlignRight);
                    previousItemLabel->setStyleSheet("padding-top: 20px; padding-right: 36px; padding-bottom: 20px; padding-left: 36px;");
                    if (i < data.items.count() - 1)
                    {
                        previousItemLabel->setCursor(Qt::PointingHandCursor);
                        const api::NewsItem &previousItem = data.items[i + 1];
                        previousItemLabel->setText(previousItem.title);
                        connect(previousItemLabel, &ClickableLabel::clicked,
                                this, [this, previousId = previousItem.id]()
                        {
                            emit navigate({Page::NEWS, previousId});
                        });
                    } else
                    {
                        previousItemLabel->setEnabled(false);
                    }
                    ui->navigationLayout->addWidget(previousItemLabel);
                    auto nextItemLabel = new ClickableLabel(ui->contentResultsScrollAreaContents);
                    nextItemLabel->setFixedHeight(58);
                    nextItemLabel->setWordWrap(true);
                    nextItemLabel->setAlignment(Qt::AlignTop | Qt::AlignLeft);
                    nextItemLabel->setStyleSheet("padding-top: 20px; padding-right: 36px; padding-bottom: 20px; padding-left: 36px;");
                    if (i > 0)
                    {
                        nextItemLabel->setCursor(Qt::PointingHandCursor);
                        const api::NewsItem &nextItem = data.items[i - 1];
                        nextItemLabel->setText(nextItem.title);
                        connect(nextItemLabel, &ClickableLabel::clicked,
                                this, [this, nextId = nextItem.id]()
                        {
                            emit navigate({Page::NEWS, nextId});
                        });
                    }
                    else
                    {
                        nextItemLabel->setEnabled(false);
                    }
                    ui->navigationLayout->addWidget(nextItemLabel);

                    ui->contentStackedWidget->setCurrentWidget(ui->contentResultsPage);
                    break;
                }
            }
        }
        else if (networkReply->error() != QNetworkReply::OperationCanceledError)
        {
            qDebug() << networkReply->error()
                     << networkReply->errorString()
                     << QString(networkReply->readAll()).toUtf8();
        }
        networkReply->deleteLater();
    });
    connect(this, &QObject::destroyed, newsPageReply, &QNetworkReply::abort);
}

void NewsPage::switchUiAuthenticatedState(bool authenticated)
{
    StoreBasePage::switchUiAuthenticatedState(authenticated);
}
