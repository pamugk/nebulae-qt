#include "releasepage.h"
#include "ui_releasepage.h"

#include <QActionGroup>
#include <QDesktopServices>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMenu>
#include <QNetworkReply>
#include <QPainter>
#include <QPushButton>
#include <QTabBar>
#include <QToolButton>

#include "../api/utils/releaseserialization.h"
#include "../api/utils/statisticsserialization.h"
#include "../api/utils/storeproductinfoserialization.h"
#include "../db/database.h"
#include "../widgets/achievementlistitem.h"
#include "../widgets/downloadlistitem.h"
#include "../widgets/imageholder.h"
#include "../widgets/videoholder.h"
#include "../windows/releasechangelogdialog.h"
#include "../windows/releasemediadialog.h"

ReleasePage::ReleasePage(QWidget *parent) :
    BasePage(parent),
    owned(false),
    platformId(),
    platformReleaseId(),
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
    auto releaseReply = apiClient->getRelease(data.toString());
    connect(releaseReply, &QNetworkReply::finished,
            this, [this, networkReply = releaseReply]()
    {
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
            getAchievements();

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
                auto storeProductReply = apiClient->getStoreProductInfo(data.externalId, locale.bcp47Name());
                connect(storeProductReply, &QNetworkReply::finished,
                        this, [this, iconUrl = data.game.squareIcon, networkReply = storeProductReply]()
                {
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

                        QVector<const api::BonusDownload *> goodies;
                        for (const api::BonusDownload &bonusDownload : std::as_const(data.mainProductInfo.downloads.bonusContent))
                        {
                            goodies << &bonusDownload;
                        }

                        QMap<QString, QSet<QString>> encounteredLanguages;
                        QMap<QString, QVector<QString>> supportedLanguages;
                        QMap<QString, QString> supportedLanguagesNames;
                        QVector<QString> supportedOperatingSystems;
                        QMap<QString, QString> supportedOperatingSystemsNames({
                                                                                  std::pair("linux", "Linux"),
                                                                                  std::pair("mac", "macOs"),
                                                                                  std::pair("windows", "Windows")
                                                                              });

                        QMap<QString, QMap<QString, QVector<api::GameDownload>>> installers;
                        for (const api::GameDownload &installerDownload : std::as_const(data.mainProductInfo.downloads.installers))
                        {
                            if (!encounteredLanguages.contains(installerDownload.os))
                            {
                                supportedOperatingSystems << installerDownload.os;
                            }
                            if (!encounteredLanguages[installerDownload.os].contains(installerDownload.language))
                            {
                                encounteredLanguages[installerDownload.os] << installerDownload.language;
                                supportedLanguages[installerDownload.os] << installerDownload.language;
                            }
                            if (!supportedLanguagesNames.contains(installerDownload.language))
                            {
                                supportedLanguagesNames[installerDownload.language] = installerDownload.languageFull;
                            }
                            installers[installerDownload.os][installerDownload.language] << installerDownload;
                        }
                        QMap<QString, QMap<QString, QVector<api::GameDownload>>> patches;
                        for (const api::GameDownload &patchDownload : std::as_const(data.mainProductInfo.downloads.patches))
                        {
                            if (!encounteredLanguages.contains(patchDownload.os))
                            {
                                supportedOperatingSystems << patchDownload.os;
                            }
                            if (!encounteredLanguages[patchDownload.os].contains(patchDownload.language))
                            {
                                encounteredLanguages[patchDownload.os] << patchDownload.language;
                                supportedLanguages[patchDownload.os] << patchDownload.language;
                            }
                            if (!supportedLanguagesNames.contains(patchDownload.language))
                            {
                                supportedLanguagesNames[patchDownload.language] = patchDownload.languageFull;
                            }
                            patches[patchDownload.os][patchDownload.language] << patchDownload;
                        }
                        QMap<QString, QMap<QString, QVector<api::GameDownload>>> languagePacks;
                        for (const api::GameDownload &languagePackDownload : std::as_const(data.mainProductInfo.downloads.languagePacks))
                        {
                            if (!encounteredLanguages.contains(languagePackDownload.os))
                            {
                                supportedOperatingSystems << languagePackDownload.os;
                            }
                            if (!encounteredLanguages[languagePackDownload.os].contains(languagePackDownload.language))
                            {
                                encounteredLanguages[languagePackDownload.os] << languagePackDownload.language;
                                supportedLanguages[languagePackDownload.os] << languagePackDownload.language;
                            }
                            if (!supportedLanguagesNames.contains(languagePackDownload.language))
                            {
                                supportedLanguagesNames[languagePackDownload.language] = languagePackDownload.languageFull;
                            }
                            languagePacks[languagePackDownload.os][languagePackDownload.language] << languagePackDownload;
                        }

                        for (const api::ProductInfo &dlc : std::as_const(data.expandedDlcs))
                        {
                            for (const api::BonusDownload &bonusDownload : std::as_const(dlc.downloads.bonusContent))
                            {
                                goodies << &bonusDownload;
                            }
                            for (const api::GameDownload &installerDownload: std::as_const(dlc.downloads.installers))
                            {
                                if (!encounteredLanguages[installerDownload.os].contains(installerDownload.language))
                                {
                                    supportedOperatingSystems << installerDownload.os;
                                }
                                if (!encounteredLanguages[installerDownload.os].contains(installerDownload.language))
                                {
                                    encounteredLanguages[installerDownload.os] << installerDownload.language;
                                    supportedLanguages[installerDownload.os] << installerDownload.language;
                                }
                                if (!supportedLanguagesNames.contains(installerDownload.language))
                                {
                                    supportedLanguagesNames[installerDownload.language] = installerDownload.languageFull;
                                }
                                installers[installerDownload.os][installerDownload.language] << installerDownload;
                            }
                            for (const api::GameDownload &patchDownload : std::as_const(dlc.downloads.patches))
                            {
                                if (!encounteredLanguages.contains(patchDownload.os))
                                {
                                    supportedOperatingSystems << patchDownload.os;
                                }
                                if (!encounteredLanguages[patchDownload.os].contains(patchDownload.language))
                                {
                                    encounteredLanguages[patchDownload.os] << patchDownload.language;
                                    supportedLanguages[patchDownload.os] << patchDownload.language;
                                }
                                if (!supportedLanguagesNames.contains(patchDownload.language))
                                {
                                    supportedLanguagesNames[patchDownload.language] = patchDownload.languageFull;
                                }
                                patches[patchDownload.os][patchDownload.language] << patchDownload;
                            }
                            for (const api::GameDownload &languagePackDownload : std::as_const(dlc.downloads.languagePacks))
                            {
                                if (!encounteredLanguages.contains(languagePackDownload.os))
                                {
                                    supportedOperatingSystems << languagePackDownload.os;
                                }
                                if (!encounteredLanguages[languagePackDownload.os].contains(languagePackDownload.language))
                                {
                                    encounteredLanguages[languagePackDownload.os] << languagePackDownload.language;
                                    supportedLanguages[languagePackDownload.os] << languagePackDownload.language;
                                }
                                if (!supportedLanguagesNames.contains(languagePackDownload.language))
                                {
                                    supportedLanguagesNames[languagePackDownload.language] = languagePackDownload.languageFull;
                                }
                                languagePacks[languagePackDownload.os][languagePackDownload.language] << languagePackDownload;
                            }
                        }

                        QFont downloadsTitleFont;
                        downloadsTitleFont.setBold(true);
                        auto locale = QLocale::system();
                        if (!goodies.isEmpty())
                        {
                            auto headerLabel = new QLabel("Goodies", ui->resultsExtrasPageScrollAreaContents);
                            headerLabel->setFont(downloadsTitleFont);
                            ui->resultsExtrasPageScrollAreaContentsLayout->addWidget(headerLabel);
                            for (const api::Download *item : std::as_const(goodies))
                            {
                                auto itemWidget = new DownloadListItem(ui->resultsExtrasPageScrollAreaContents);
                                itemWidget->setCursor(Qt::PointingHandCursor);
                                itemWidget->setSize(locale.formattedDataSize(item->totalSize, 1, QLocale::DataSizeTraditionalFormat));
                                itemWidget->setTitle(item->name);
                                ui->resultsExtrasPageScrollAreaContentsLayout->addWidget(itemWidget);
                            }
                        }
                        if (!installers.isEmpty())
                        {
                            if (!ui->resultsExtrasPageScrollAreaContentsLayout->isEmpty())
                            {
                                ui->resultsExtrasPageScrollAreaContentsLayout->addSpacing(24);
                            }

                            QString systemLanguage = locale.languageToCode(locale.language(), QLocale::LanguageCodeType::ISO639Alpha2);
                            QString preferredOs;
#ifdef Q_OS_WINDOWS
                            preferredOs = "windows";
#endif
#ifdef Q_OS_MACOS
                            preferredOs = "mac";
#endif
#ifdef Q_OS_LINUX
                            preferredOs = "linux";
#endif

                            if (!installers.contains(preferredOs))
                            {
                                preferredOs = supportedOperatingSystems[0];
                            }

                            auto headerWidget = new QWidget(ui->resultsExtrasPageScrollAreaContents);
                            auto headerLayout = new QHBoxLayout;
                            headerWidget->setLayout(headerLayout);
                            auto headerLabel = new QLabel("Offline backup installers", headerWidget);
                            headerLabel->setFont(downloadsTitleFont);
                            headerLayout->addWidget(headerLabel);

                            auto editionsToolButton = new QToolButton(headerWidget);
                            headerLayout->addWidget(editionsToolButton);
                            headerLayout->addStretch();
                            ui->resultsExtrasPageScrollAreaContentsLayout->addWidget(headerWidget);
                            auto editionsMenu = new QMenu(editionsToolButton);
                            editionsToolButton->setMenu(editionsMenu);
                            editionsToolButton->setPopupMode(QToolButton::MenuButtonPopup);

                            QMap<QString, QAction *> osActions;
                            auto osActionsGroup = new QActionGroup(editionsMenu);
                            for (const auto &os : std::as_const(supportedOperatingSystems))
                            {
                                auto osAction = editionsMenu->addAction(supportedOperatingSystemsNames.value(os, "Unknown OS"));
                                osAction->setCheckable(true);
                                osAction->setChecked(os == preferredOs);
                                osActionsGroup->addAction(osAction);
                                osActions[os] = osAction;

                                connect(osAction, &QAction::toggled, editionsToolButton,
                                        [editionsToolButton, os, supportedLanguagesNames, supportedOperatingSystemsNames](bool toggled)
                                {
                                   if (toggled)
                                   {
                                       editionsToolButton->setProperty("os", os);
                                       auto language = editionsToolButton->property("language").toMap()[os].toString();
                                       editionsToolButton->setText(QString("%1, %2").arg(supportedOperatingSystemsNames[os],
                                                                                         supportedLanguagesNames[language]));
                                   }
                                });
                            }
                            editionsMenu->addSeparator();

                            QVariantMap selectedLanguages;
                            for (const auto [os, osSupportedLanguages] : supportedLanguages.asKeyValueRange())
                            {
                                bool isPreferredOs = os == preferredOs;
                                auto osAction = osActions[os];
                                QString preferredLanguage;
                                if (installers[os].contains(systemLanguage))
                                {
                                    preferredLanguage = systemLanguage;
                                }
                                else
                                {
                                    preferredLanguage = installers[os].contains("en") ? "en" : osSupportedLanguages[0];
                                }
                                selectedLanguages[os] = preferredLanguage;

                                auto osLanguagesActionGroup = new QActionGroup(editionsMenu);
                                for (const auto &language : std::as_const(osSupportedLanguages))
                                {
                                    auto isPreferredLanguage = language == preferredLanguage;
                                    auto languageAction = editionsMenu->addAction(supportedLanguagesNames.value(language));
                                    languageAction->setCheckable(true);
                                    languageAction->setChecked(isPreferredLanguage);
                                    languageAction->setVisible(isPreferredOs);
                                    osLanguagesActionGroup->addAction(languageAction);

                                    connect(osAction, &QAction::toggled, languageAction, &QAction::setVisible);
                                    connect(languageAction, &QAction::toggled, editionsToolButton,
                                            [editionsToolButton, language, supportedLanguagesNames, supportedOperatingSystemsNames](bool toggled)
                                    {
                                       if (toggled)
                                       {
                                           auto os = editionsToolButton->property("os").toString();
                                           editionsToolButton->property("language").toMap()[os] = language;
                                           editionsToolButton->setText(QString("%1, %2").arg(supportedOperatingSystemsNames[os],
                                                                                             supportedLanguagesNames[language]));
                                       }
                                    });

                                    if (installers.contains(os) && installers[os].contains(language))
                                    {
                                        for (const api::GameDownload &item : std::as_const(installers[os][language]))
                                        {
                                            auto itemWidget = new DownloadListItem(ui->resultsExtrasPageScrollAreaContents);
                                            itemWidget->setCursor(Qt::PointingHandCursor);
                                            itemWidget->setSize(locale.formattedDataSize(item.totalSize, 1, QLocale::DataSizeTraditionalFormat));
                                            itemWidget->setTitle(item.name);
                                            itemWidget->setVersion(item.version);
                                            itemWidget->setVisible(isPreferredOs && isPreferredLanguage);
                                            ui->resultsExtrasPageScrollAreaContentsLayout->addWidget(itemWidget);

                                            connect(languageAction, &QAction::toggled, itemWidget, [itemWidget, osAction](bool toggled)
                                            {
                                                itemWidget->setVisible(toggled && osAction->isChecked());
                                            });
                                            connect(osAction, &QAction::toggled, itemWidget, [itemWidget, languageAction](bool toggled)
                                            {
                                                itemWidget->setVisible(toggled && languageAction->isChecked());
                                            });
                                        }
                                    }
                                    if (languagePacks.contains(os) && languagePacks[os].contains(language))
                                    {
                                        for (const api::GameDownload &item : std::as_const(languagePacks[os][language]))
                                        {
                                            auto itemWidget = new DownloadListItem(ui->resultsExtrasPageScrollAreaContents);
                                            itemWidget->setCursor(Qt::PointingHandCursor);
                                            itemWidget->setSize(locale.formattedDataSize(item.totalSize, 1, QLocale::DataSizeTraditionalFormat));
                                            itemWidget->setTitle(item.name);
                                            itemWidget->setVersion(item.version);
                                            itemWidget->setVisible(isPreferredOs && isPreferredLanguage);
                                            ui->resultsExtrasPageScrollAreaContentsLayout->addWidget(itemWidget);

                                            connect(languageAction, &QAction::toggled, itemWidget, [itemWidget, osAction](bool toggled)
                                            {
                                                itemWidget->setVisible(toggled && osAction->isChecked());
                                            });
                                            connect(osAction, &QAction::toggled, itemWidget, [itemWidget, languageAction](bool toggled)
                                            {
                                                itemWidget->setVisible(toggled && languageAction->isChecked());
                                            });
                                        }
                                    }
                                    if (patches.contains(os) && patches[os].contains(language))
                                    {
                                        for (const api::GameDownload &item : std::as_const(patches[os][language]))
                                        {
                                            auto itemWidget = new DownloadListItem(ui->resultsExtrasPageScrollAreaContents);
                                            itemWidget->setCursor(Qt::PointingHandCursor);
                                            itemWidget->setSize(locale.formattedDataSize(item.totalSize, 1, QLocale::DataSizeTraditionalFormat));
                                            itemWidget->setTitle(item.name);
                                            itemWidget->setVersion(item.version);
                                            itemWidget->setVisible(isPreferredOs && isPreferredLanguage);
                                            ui->resultsExtrasPageScrollAreaContentsLayout->addWidget(itemWidget);

                                            connect(languageAction, &QAction::toggled, itemWidget, [itemWidget, osAction](bool toggled)
                                            {
                                                itemWidget->setVisible(toggled && osAction->isChecked());
                                            });
                                            connect(osAction, &QAction::toggled, itemWidget, [itemWidget, languageAction](bool toggled)
                                            {
                                                itemWidget->setVisible(toggled && languageAction->isChecked());
                                            });
                                        }
                                    }
                                }
                            }

                            editionsToolButton->setProperty("os", preferredOs);
                            editionsToolButton->setProperty("language", selectedLanguages);
                            editionsToolButton->setText(QString("%1, %2").arg(supportedOperatingSystemsNames[preferredOs],
                                                                              supportedLanguagesNames[selectedLanguages[preferredOs].toString()]));
                        }

                        if (!ui->resultsExtrasPageScrollAreaContentsLayout->isEmpty())
                        {
                            auto sectionsTabBar = static_cast<QTabBar *>(uiActions[0]);
                            sectionsTabBar->setTabVisible(2, owned);
                            sectionsTabBar->setMinimumWidth(owned ? 300 : 200);
                            ui->resultsExtrasPageScrollAreaContentsLayout->addStretch();
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
                connect(this, &QObject::destroyed, storeProductReply, &QNetworkReply::abort);
            }

            if (!data.game.horizontalArtwork.isEmpty())
            {
                auto horizontalArtworkReply = apiClient->getAnything(QString(data.game.horizontalArtwork).replace("{formatter}", "_glx_bg_top_padding_7").replace("{ext}", "webp"));
                connect(horizontalArtworkReply, &QNetworkReply::finished, this, [this, horizontalArtworkReply]()
                {
                    if (horizontalArtworkReply->error() == QNetworkReply::NoError)
                    {
                        horizontalArtwork.loadFromData(horizontalArtworkReply->readAll());
                        repaint();
                    }
                    else if (horizontalArtworkReply->error() != QNetworkReply::OperationCanceledError)
                    {
                        qDebug() << horizontalArtworkReply->error()
                                 << horizontalArtworkReply->errorString()
                                 << QString(horizontalArtworkReply->readAll()).toUtf8();
                    }

                    horizontalArtworkReply->deleteLater();
                });
                connect(this, &QObject::destroyed, horizontalArtworkReply, &QNetworkReply::abort);
            }
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
    connect(this, &QObject::destroyed, releaseReply, &QNetworkReply::abort);
}

void ReleasePage::paintEvent(QPaintEvent *event)
{
    QPainter backgroundPainter(this);
    if (!horizontalArtwork.isNull())
    {
        auto scaledBackground = horizontalArtwork.scaledToWidth(width());
        backgroundPainter.drawPixmap(0, 0, scaledBackground);
        auto remainingHeight = height() - scaledBackground.height();
        // TODO: implement neat gradient based on image colors
        if (remainingHeight > 0)
        {
            backgroundPainter.fillRect(0, scaledBackground.height(), width(), remainingHeight, Qt::black);
        }
    }

    backgroundPainter.fillRect(rect(), QColor(128, 128, 128, 128));
    QWidget::paintEvent(event);
}

void ReleasePage::switchUiAuthenticatedState(bool authenticated)
{
    emit uiAuthenticatedSwitchRequested();
    ui->achievementsProgressLabel->setText("Achievements N/A");
    ui->gametimeLabel->setText("Game time N/A");
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

    auto sectionsTabBar = static_cast<QTabBar *>(uiActions[0]);
    sectionsTabBar->setTabVisible(2, owned && !ui->resultsExtrasPageScrollAreaContentsLayout->isEmpty());
    sectionsTabBar->setMinimumWidth(sectionsTabBar->isTabVisible(2) ? 300 : 200);

    getAchievements();
}

void ReleasePage::openGalleryOnItem(std::size_t index)
{
    ReleaseMediaDialog dialog(videos, screenshots, apiClient, this);
    dialog.viewMedia(index);
    dialog.exec();
}

void ReleasePage::getAchievements()
{
    if (!platformId.isNull())
    {
        while (!ui->resultsProgressPageScrollContentsLayout->isEmpty())
        {
            auto item = ui->resultsProgressPageScrollContentsLayout->layout()->itemAt(0);
            ui->resultsProgressPageScrollContentsLayout->layout()->removeItem(item);
            item->widget()->deleteLater();
            delete item;
        }

        QNetworkReply *releaseAchievementsReply;
        if (apiClient->isAuthenticated())
        {
            releaseAchievementsReply = apiClient->getCurrentUserPlatformReleaseAchievements(platformId, platformReleaseId, QString());
        }
        else
        {
            releaseAchievementsReply = apiClient->getPlatformReleaseAchievements(platformId, platformReleaseId,
                                                                                 "en-US");
        }
        connect(releaseAchievementsReply, &QNetworkReply::finished,
                this, [this, networkReply = releaseAchievementsReply]()
        {
            if (networkReply->error() == QNetworkReply::NoError)
            {
                auto locale = QLocale::system();
                auto resultJson = QJsonDocument::fromJson(QString(networkReply->readAll()).toUtf8()).object();
                api::GetUserPlatformAchievementsResponse data;
                parseGetUserPlatformAchievementsResponse(resultJson, data);
                if (data.items.isEmpty())
                {
                    ui->resultsProgressPageScrollArea->setVisible(false);
                }
                else
                {
                    ui->resultsProgressPageScrollArea->setVisible(true);
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
        connect(this, &ReleasePage::uiAuthenticatedSwitchRequested, releaseAchievementsReply, &QNetworkReply::abort);
        connect(this, &QObject::destroyed, releaseAchievementsReply, &QNetworkReply::abort);
    }
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

        auto releaseGametimeStatisticsReply = apiClient->getCurrentUserPlatformReleaseGameTimeStatistics(platformId, platformReleaseId);
        connect(releaseGametimeStatisticsReply, &QNetworkReply::finished,
                this, [this, networkReply = releaseGametimeStatisticsReply]()
        {
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
            }
            else if (networkReply->error() != QNetworkReply::OperationCanceledError)
            {
                qDebug() << networkReply->error()
                         << networkReply->errorString()
                         << QString(networkReply->readAll()).toUtf8();
            }

            networkReply->deleteLater();
        });
        connect(this, &ReleasePage::uiAuthenticatedSwitchRequested, releaseGametimeStatisticsReply, &QNetworkReply::abort);
        connect(this, &QObject::destroyed, releaseGametimeStatisticsReply, &QNetworkReply::abort);
    }
    else
    {
        owned = false;
        ui->ratingLabel->setText(QString());
        ui->userTagsLabel->setText(QString());
    }
}
