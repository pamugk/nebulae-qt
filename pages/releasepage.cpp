#include "releasepage.h"
#include "ui_releasepage.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QMenu>
#include <QNetworkReply>
#include <QPushButton>
#include <QTabBar>
#include <QToolButton>

#include "../api/utils/statisticsserialization.h"
#include "../api/utils/releaseserialization.h"
#include "../widgets/achievementlistitem.h"
#include "../widgets/imageholder.h"
#include "../widgets/videoholder.h"
#include "../windows/releasemediadialog.h"

ReleasePage::ReleasePage(QWidget *parent) :
    BasePage(parent),
    releaseAchievementsReply(nullptr),
    releaseGametimeStatisticsReply(nullptr),
    releaseReply(nullptr),
    ui(new Ui::ReleasePage)
{
    ui->setupUi(this);

    auto sectionsTabBar = new QTabBar(this);
    sectionsTabBar->addTab("Overview");
    sectionsTabBar->addTab("My progress");
    sectionsTabBar->addTab("Extras");
    sectionsTabBar->setTabVisible(2, false);
    sectionsTabBar->setVisible(false);
    connect(sectionsTabBar, &QTabBar::currentChanged, this, [this](int index)
    {
        switch (index)
        {
        case 0:
            ui->resultsStack->setCurrentWidget(ui->resultsOverviewPage);
            break;
        case 1:
            ui->resultsStack->setCurrentWidget(ui->resultsProgressPage);
            break;
        case 2:
            ui->resultsStack->setCurrentWidget(ui->resultsExtrasPage);
            break;
        default:
            break;
        }
    });
    uiActions << sectionsTabBar;

    auto markAsOwnedButton = new QPushButton("Mark as owned", this);
    markAsOwnedButton->setVisible(false);
    uiActions << markAsOwnedButton;

    auto installButton = new QPushButton("Install", this);
    installButton->setIcon(QIcon(":/icons/download.svg"));
    installButton->setVisible(false);
    uiActions << installButton;

    auto detailsToolButton = new QToolButton(this);
    detailsToolButton->setIcon(QIcon(":icons/sliders.svg"));
    detailsToolButton->setPopupMode(QToolButton::InstantPopup);
    detailsToolButton->setVisible(false);

    auto detailsMenu = new QMenu(detailsToolButton);
    detailsMenu->addAction("View patch notes");
    detailsMenu->addAction("Edit…")->setVisible(false);
    detailsMenu->addSeparator()->setVisible(false);
    auto installationManagementMenu = detailsMenu->addMenu("Manage installation");
    installationManagementMenu->addAction("Configure…");
    installationManagementMenu->addSeparator();
    installationManagementMenu->addAction("Import folder");
    installationManagementMenu->addAction("Show folder");
    installationManagementMenu->addAction("Show screenshots");
    installationManagementMenu->addAction("Verify / Repair");
    installationManagementMenu->addSeparator();
    installationManagementMenu->addAction("Uninstall");
    installationManagementMenu->menuAction()->setVisible(false);
    detailsMenu->addAction("Check for updates")->setVisible(false);
    detailsMenu->addSeparator();
    QIcon externalLinkIcon(":/icons/up-right-from-square.svg");
    detailsMenu->addAction("Get support", detailsMenu, [this]()
    {

    })->setIcon(externalLinkIcon);
    detailsMenu->addAction("View game forum", detailsMenu, [this]()
    {

    })->setIcon(externalLinkIcon);
    detailsMenu->addAction("View in store", detailsMenu, [this]()
    {

    });

    detailsToolButton->setMenu(detailsMenu);
    uiActions << detailsToolButton;

}

ReleasePage::~ReleasePage()
{
    if (releaseAchievementsReply != nullptr)
    {
        releaseAchievementsReply->abort();
    }
    if (releaseGametimeStatisticsReply != nullptr)
    {
        releaseGametimeStatisticsReply->abort();
    }
    if (releaseReply != nullptr)
    {
        releaseReply->abort();
    }
    delete ui;
}

const QVector<QWidget *> ReleasePage::getHeaderControls()
{
    return uiActions;
}

void ReleasePage::setApiClient(api::GogApiClient *apiClient)
{
    this->apiClient = apiClient;
}

void ReleasePage::initialize(const QVariant &data)
{
    ui->contentsStack->setCurrentWidget(ui->loaderPage);
    releaseReply = apiClient->getRelease(data.toString());
    connect(releaseReply, &QNetworkReply::finished, this, [this](){
        auto networkReply = releaseReply;
        releaseReply = nullptr;

        if (networkReply->error() == QNetworkReply::NoError)
        {
            auto resultJson = QJsonDocument::fromJson(QString(networkReply->readAll()).toUtf8()).object();
            api::Release data;
            parseRelease(resultJson, data);

            if (data.platformId == "gog")
            {
                ui->platformLabel->setText("GOG.com");
            }
            else
            {
                ui->platformLabel->setText("Other");
            }

            std::size_t mediaIndex = 0;
            for (const auto &video : std::as_const(data.videos))
            {
                QString videoThumbnailLink;
                if (video.provider == "youtube")
                {
                    videoThumbnailLink = QString("https://img.youtube.com/vi/%1/mqdefault.jpg").arg(video.videoId);
                }
                if (!videoThumbnailLink.isNull())
                {
                    auto videoHolder = new VideoHolder(QSize(200, 120), videoThumbnailLink, apiClient, ui->mediaScrollAreaContents);
                    videoHolder->setCursor(Qt::PointingHandCursor);
                    connect(videoHolder, &VideoHolder::clicked, this, [this, mediaIndex]()
                    {
                        openGalleryOnItem(mediaIndex);
                    });
                    ui->mediaScrollAreaContentsLayout->addWidget(videoHolder);
                    videos << video;
                    mediaIndex++;
                }
            }
            for (const auto &screenshotLink : std::as_const(data.game.screenshots))
            {
                QString formattedLink = QString(screenshotLink).replace("{formatter}", "_glx_logo_2x").replace("{ext}", "webp");
                auto screenshotHolder = new ImageHolder(QSize(200, 120), formattedLink, apiClient, ui->mediaScrollAreaContents);
                screenshotHolder->setCursor(Qt::PointingHandCursor);
                connect(screenshotHolder, &ImageHolder::clicked, this, [this, mediaIndex]()
                {
                    openGalleryOnItem(mediaIndex);
                });
                ui->mediaScrollAreaContentsLayout->addWidget(screenshotHolder);
                screenshots << screenshotLink;
                mediaIndex++;
            }
            ui->mediaScrollArea->setVisible(mediaIndex > 0);

            auto locale = QLocale::system();
            ui->titleLabel->setText(data.title["*"].toString());
            ui->descriptionLabel->setText(data.summary["*"].toString());
            if (data.game.aggregatedRating.has_value())
            {
                ui->criticsLabel->setText(QString("<b>Critics:</b> %1 / 100").arg(locale.toString(std::round(data.game.aggregatedRating.value()))));
            }
            else
            {
                ui->criticsLabel->setText("<b>Critics:</b> -");
            }
            if (data.game.genres.isEmpty())
            {
                ui->genreLabel->setText("<b>Genre:</b> -");
            }
            else
            {
                QStringList genreNames;
                genreNames.reserve(data.game.genres.count());
                for (const api::LocalizedMetaTag &genre : std::as_const(data.game.genres))
                {
                    genreNames << genre.name["*"].toString();
                }
                ui->genreLabel->setText(QString("<b>Genre:</b> %1").arg(locale.createSeparatedList(genreNames)));
            }
            if (data.game.themes.isEmpty())
            {
                ui->themeLabel->setText("<b>Style:</b> -");
            }
            else
            {
                QStringList themeNames;
                themeNames.reserve(data.game.themes.count());
                for (const api::LocalizedMetaTag &theme : std::as_const(data.game.themes))
                {
                    themeNames << theme.name["*"].toString();
                }
                ui->themeLabel->setText(QString("<b>Style:</b> %1").arg(locale.createSeparatedList(themeNames)));
            }
            if (data.firstReleaseDate.isNull())
            {
                ui->releaseDateLabel->setText("<b>Release date:</b> -");
            }
            else
            {
                ui->releaseDateLabel->setText(QString("<b>Release date:</b> %1").arg(locale.toString(data.firstReleaseDate.date())));
            }
            if (data.game.developers.isEmpty())
            {
                ui->developerLabel->setText("<b>Developer:</b> -");
            }
            else
            {
                QStringList developerNames;
                developerNames.reserve(data.game.developers.count());
                for (const api::IdMetaTag &developer : std::as_const(data.game.developers))
                {
                    developerNames << developer.name;
                }
                ui->developerLabel->setText(QString("<b>Developer:</b> %1").arg(locale.createSeparatedList(developerNames)));
            }

            uiActions[0]->setVisible(true);
            ui->resultsStack->setCurrentWidget(ui->resultsOverviewPage);
            ui->contentsStack->setCurrentWidget(ui->resultsPage);

            releaseAchievementsReply = apiClient->getCurrentUserPlatformReleaseAchievements(data.platformId, data.externalId, QString());
            connect(releaseAchievementsReply, &QNetworkReply::finished, this, [this]()
            {
                auto networkReply = releaseAchievementsReply;
                releaseAchievementsReply = nullptr;
                if (networkReply->error() == QNetworkReply::NoError)
                {
                    auto locale = QLocale::system();
                    auto resultJson = QJsonDocument::fromJson(QString(networkReply->readAll()).toUtf8()).object();
                    api::GetUserPlatformAchievementsResponse data;
                    parseGetUserPlatformAchievementsResponse(resultJson, data);
                    if (data.items.isEmpty())
                    {
                        ui->resultsProgressPageScrollArea->setVisible(false);
                        ui->resultsPageLayout->addStretch();
                    }
                    else
                    {
                        int unlockedAchievements = 0;
                        std::sort(data.items.begin(), data.items.end(),
                                  [](const api::PlatformUserAchievement &a, const api::PlatformUserAchievement &b)
                        {
                            return a.dateUnlocked == b.dateUnlocked && a.name < b.name || a.dateUnlocked > b.dateUnlocked;
                        });
                        for (const api::PlatformUserAchievement &achievement : std::as_const(data.items))
                        {
                            unlockedAchievements += achievement.dateUnlocked.isNull() ? 0 : 1;
                            auto achievementItem = new AchievementListItem(achievement, achievement.dateUnlocked.date(), apiClient);
                            ui->resultsProgressPageScrollContentsLayout->addWidget(achievementItem);
                        }
                        ui->resultsProgressPageScrollContentsLayout->addStretch();
                        ui->achievementsProgressLabel->setText(QString("Achievements %1%2")
                                                               .arg(locale.toString(std::round(100. * unlockedAchievements / data.items.count())), locale.percent()));
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

            releaseGametimeStatisticsReply = apiClient->getCurrentUserPlatformReleaseGameTimeStatistics(data.platformId, data.externalId);
            connect(releaseGametimeStatisticsReply, &QNetworkReply::finished, this, [this]()
            {
                auto networkReply = releaseGametimeStatisticsReply;
                releaseGametimeStatisticsReply = nullptr;
                if (networkReply->error() == QNetworkReply::NoError)
                {
                    auto resultJson = QJsonDocument::fromJson(QString(networkReply->readAll()).toUtf8());
                    if (!resultJson["time_sum"].isNull())
                    {
                        int totalPlaytime = resultJson["time_sum"].toInt();
                        auto locale = QLocale::system();
                        QString gametimeText("Game time ");
                        if (totalPlaytime > 60)
                        {
                            gametimeText += locale.toString(totalPlaytime / 60);
                            gametimeText += " h. ";
                        }
                        gametimeText += locale.toString(totalPlaytime % 60);
                        gametimeText += " min.";
                        ui->gametimeLabel->setText(gametimeText);
                    }
                    QDateTime lastPlayed;
                    if (!resultJson["last_session_date"].isNull())
                    {
                        lastPlayed = QDateTime::fromSecsSinceEpoch(resultJson["last_session_date"].toInteger());
                    }
                    // TODO: process statistics
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
        else if (networkReply->error() == QNetworkReply::ContentNotFoundError)
        {
            ui->contentsStack->setCurrentWidget(ui->emptyPage);
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

void ReleasePage::switchUiAuthenticatedState(bool authenticated)
{

}

void ReleasePage::openGalleryOnItem(std::size_t index)
{
    ReleaseMediaDialog dialog(videos, screenshots, apiClient, this);
    dialog.viewMedia(index);
    dialog.exec();
}
