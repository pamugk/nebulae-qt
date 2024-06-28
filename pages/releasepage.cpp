#include "releasepage.h"
#include "ui_releasepage.h"

#include <QDesktopServices>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMenu>
#include <QNetworkReply>
#include <QPushButton>
#include <QTabBar>
#include <QToolButton>

#include "../api/utils/releaseserialization.h"
#include "../api/utils/statisticsserialization.h"
#include "../api/utils/storeproductinfoserialization.h"
#include "../db/database.h"
#include "../widgets/achievementlistitem.h"
#include "../widgets/imageholder.h"
#include "../widgets/videoholder.h"
#include "../windows/releasechangelogdialog.h"
#include "../windows/releasemediadialog.h"

ReleasePage::ReleasePage(QWidget *parent) :
    BasePage(parent),
    owned(false),
    platformId(),
    platformReleaseId(),
    releaseAchievementsReply(nullptr),
    releaseGametimeStatisticsReply(nullptr),
    releaseReply(nullptr),
    storeProductReply(nullptr),
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
    detailsMenu->addAction("View patch notes")->setVisible(false);
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
    detailsMenu->addSeparator()->setVisible(false);
    QIcon externalLinkIcon(":/icons/up-right-from-square.svg");
    auto supportAction = detailsMenu->addAction("Get support");
    supportAction->setIcon(externalLinkIcon);
    supportAction->setVisible(false);

    auto gameForumAction = detailsMenu->addAction("View game forum");
    gameForumAction->setIcon(externalLinkIcon);
    gameForumAction->setVisible(false);

    detailsMenu->addAction("View in store", detailsMenu, [this]()
    {
        emit navigate({ CATALOG_PRODUCT, platformReleaseId });
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
    if (storeProductReply != nullptr)
    {
        storeProductReply->abort();
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

            platformId = data.platformId;
            platformReleaseId = data.externalId;
            if (!apiClient->currentUserId().isNull())
            {
                updateUserReleaseInfo();
            }

            if (data.platformId == "gog")
            {
                uiActions[3]->setVisible(true);
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
            ui->mediaScrollAreaContentsLayout->addStretch();
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

            if (data.platformId == "gog")
            {
                storeProductReply = apiClient->getStoreProductInfo(data.externalId, locale.bcp47Name());
                connect(storeProductReply, &QNetworkReply::finished, this, [this, iconUrl = data.game.squareIcon]()
                {
                    auto networkReply = storeProductReply;
                    storeProductReply = nullptr;
                    if (networkReply->error() == QNetworkReply::NoError)
                    {
                        auto resultJson = QJsonDocument::fromJson(QString(networkReply->readAll()).toUtf8()).object();
                        api::GetStoreProductInfoResponse data;
                        parseStoreOwnedProductInfoResponse(resultJson, data);

                        auto detailsToolButton = static_cast<QToolButton *>(uiActions[3]);
                        auto detailsMenu = detailsToolButton->menu();
                        auto detailsActions = detailsMenu->actions();
                        if (!data.changelog.isEmpty())
                        {
                            auto changelogAction = detailsActions[0];
                            connect(changelogAction, &QAction::triggered, detailsMenu, [this, changelog = data.changelog, iconUrl]()
                            {
                                QString formattedIconUrl = QString(iconUrl).replace("{formatter}", "_glx_square_icon_v2").replace("{ext}", "webp");
                                ReleaseChangelogDialog dialog(changelog, ui->titleLabel->text(), "GOG.com", formattedIconUrl,
                                                              this->apiClient, this);
                                dialog.exec();
                            });
                            changelogAction->setVisible(true);
                            detailsActions[2]->setVisible(true);
                        }
                        if (!data.mainProductInfo.supportLink.isNull())
                        {
                            auto supportAction = detailsActions[6];
                            connect(supportAction, &QAction::triggered, detailsMenu, [supportUrl = data.mainProductInfo.supportLink]()
                            {
                                QDesktopServices::openUrl(supportUrl);
                            });
                            supportAction->setVisible(true);
                        }
                        if (!data.mainProductInfo.forumLink.isNull())
                        {
                            auto supportAction = detailsActions[7];
                            connect(supportAction, &QAction::triggered, detailsMenu, [forumUrl = data.mainProductInfo.forumLink]()
                            {
                                QDesktopServices::openUrl(forumUrl);
                            });
                            supportAction->setVisible(true);
                        }

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
                            ui->resultsExtrasPageScrollAreaContentsLayout->addWidget(new QLabel("Goodies", ui->resultsExtrasPageScrollAreaContents));
                            for (const api::Download *item : std::as_const(goodies))
                            {
                                ui->resultsExtrasPageScrollAreaContentsLayout->addWidget(new QLabel(item->name, ui->resultsExtrasPageScrollAreaContents));
                            }
                            showDownloads = true;
                        }
                        if (!installers.isEmpty())
                        {
                            ui->resultsExtrasPageScrollAreaContentsLayout->addWidget(new QLabel("Offline backup installers", ui->resultsExtrasPageScrollAreaContents));
                            for (const api::Download *item : std::as_const(installers))
                            {
                                ui->resultsExtrasPageScrollAreaContentsLayout->addWidget(new QLabel(item->name, ui->resultsExtrasPageScrollAreaContents));
                            }
                            for (const api::Download *item : std::as_const(languagePacks))
                            {
                                ui->resultsExtrasPageScrollAreaContentsLayout->addWidget(new QLabel(item->name, ui->resultsExtrasPageScrollAreaContents));
                            }
                            for (const api::Download *item : std::as_const(patches))
                            {
                                ui->resultsExtrasPageScrollAreaContentsLayout->addWidget(new QLabel(item->name, ui->resultsExtrasPageScrollAreaContents));
                            }
                            showDownloads = true;
                        }

                        auto sectionsToolBar = static_cast<QTabBar *>(uiActions[0]);
                        sectionsToolBar->setTabVisible(2, owned && !ui->resultsExtrasPageScrollAreaContentsLayout->isEmpty());
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
    // TODO: reset / fetch achievements & game time statistics
    if (authenticated)
    {
        if (!platformId.isNull())
        {
            updateUserReleaseInfo();
        }
    }
    else
    {
        owned = false;
        ui->ratingLabel->setText(QString());
        ui->userTagsLabel->setText(QString());
    }

    auto sectionsToolBar = static_cast<QTabBar *>(uiActions[0]);
    sectionsToolBar->setTabVisible(2, owned && !ui->resultsExtrasPageScrollAreaContentsLayout->isEmpty());
}

void ReleasePage::openGalleryOnItem(std::size_t index)
{
    ReleaseMediaDialog dialog(videos, screenshots, apiClient, this);
    dialog.viewMedia(index);
    dialog.exec();
}

void ReleasePage::updateUserReleaseInfo()
{
    auto userRelease = db::getUserRelease(apiClient->currentUserId(), platformId, platformReleaseId);
    if (userRelease.has_value())
    {
        owned = true;
        const api::UserRelease &userReleaseData = userRelease.value();
        if (userReleaseData.rating.has_value())
        {
            ui->ratingLabel->setText(QString(userReleaseData.rating.value(), u'★') + QString(5 - userReleaseData.rating.value(), u'☆'));
        }
        else
        {
            ui->ratingLabel->setText("No rating");
        }
        ui->userTagsLabel->setText(QLocale::system().createSeparatedList(userReleaseData.tags));
    }
    else
    {
        owned = false;
        ui->ratingLabel->setText(QString());
        ui->userTagsLabel->setText(QString());
    }
}
