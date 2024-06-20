#include "catalogproductpage.h"
#include "ui_catalogproductpage.h"

#include <algorithm>

#include <QButtonGroup>
#include <QCheckBox>
#include <QJsonDocument>
#include <QLocale>
#include <QPainter>
#include <QRadioButton>
#include <QNetworkReply>
#include <QScrollBar>

#include "../api/utils/catalogproductserialization.h"
#include "../api/utils/priceserialization.h"
#include "../api/utils/recommendationserialization.h"
#include "../api/utils/reviewserialization.h"
#include "../api/utils/seriesgameserialization.h"
#include "../internals/regionalutils.h"
#include "../internals/tagutils.h"
#include "../layouts/flowlayout.h"
#include "../widgets/bonusitem.h"
#include "../widgets/checkeditem.h"
#include "../widgets/clickablelabel.h"
#include "../widgets/collapsiblearea.h"
#include "../widgets/dependentproductitem.h"
#include "../widgets/featureitem.h"
#include "../widgets/imageholder.h"
#include "../widgets/pagination.h"
#include "../widgets/reviewitem.h"
#include "../widgets/simpleproductitem.h"
#include "../widgets/videoholder.h"
#include "../windows/catalogproductmediadialog.h"

CatalogProductPage::CatalogProductPage(QWidget *parent) :
    StoreBasePage(Page::CATALOG_PRODUCT, parent),
    averageRatingReply(nullptr),
    averageOwnerRatingReply(nullptr),
    backgroundReply(nullptr),
    lastReviewsReply(nullptr),
    logotypeReply(nullptr),
    mainReply(nullptr),
    ownedProductsReply(nullptr),
    pricesReply(nullptr),
    recommendedPurchasedTogetherReply(nullptr),
    recommendedSimilarReply(nullptr),
    reviewFilters({ { "en-US", "ru-RU", "de-DE", "pl-PL", "fr-FR", "zh-Hans" } }),
    reviewsPage(1),
    reviewsPageSize(5),
    reviewsOrder({"votes", false}),
    seriesGamesReply(nullptr),
    seriesTotalPriceReply(nullptr),
    wishlistReply(nullptr),
    ui(new Ui::CatalogProductPage)
{
    ui->setupUi(this);

    ui->contentRatingWarningPageLayout->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);

    ui->starLabel->setVisible(false);
    ui->ratingLabel->setVisible(false);

    ui->discountLabel->setVisible(false);
    ui->oldPriceLabel->setVisible(false);
    ui->pricelabel->setVisible(false);

    ui->windowsLabel->setVisible(false);
    ui->macOsLabel->setVisible(false);
    ui->linuxLabel->setVisible(false);

    ui->productPriceLayout->setAlignment(Qt::AlignTop);
    ui->gameFeaturesLayout->setAlignment(Qt::AlignTop);
    ui->languagesLayout->setAlignment(Qt::AlignTop);

    ui->genreHolder->setLayout(new FlowLayout(ui->genreHolder, 0, 13, 7));
    ui->tagsHolder->setLayout(new FlowLayout(ui->tagsHolder, 0, 4, 7));
    ui->companyHolder->setLayout(new FlowLayout(ui->companyHolder, 0, 14, 7));
    ui->productGoodiesSection->setLayout(new FlowLayout(ui->productGoodiesSection, -1, 15, 12));

    ui->systemRequirementsTabWidget->setTabVisible(ui->systemRequirementsTabWidget->indexOf(ui->windowsTab), false);
    ui->windowsTabLayout->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    ui->systemRequirementsTabWidget->setTabVisible(ui->systemRequirementsTabWidget->indexOf(ui->macOsTab), false);
    ui->macOsTabLayout->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    ui->systemRequirementsTabWidget->setTabVisible(ui->systemRequirementsTabWidget->indexOf(ui->linuxTab), false);
    ui->linuxTabLayout->setAlignment(Qt::AlignLeft | Qt::AlignTop);

    ui->seriesResultPage->setLayout(new FlowLayout(ui->seriesResultPage, -1, 24, 24));
    ui->similarProductsResultPage->setLayout(new FlowLayout(ui->similarProductsResultPage, -1, 24, 24));
    ui->purchasedTogetherResultPage->setLayout(new FlowLayout(ui->purchasedTogetherResultPage, -1, 24, 24));

    ui->criticsReviewLabel->setVisible(false);

    ui->userReviewsLabel->setVisible(false);
    ui->userReviewsHeader->setVisible(false);
    ui->ownersRatingLabel->setVisible(false);
    ui->userReviewsStackedWidget->setVisible(false);
    ui->userReviewsLoaderPageLayout->setAlignment(Qt::AlignTop);
    ui->userReviewsContentsLayout->setAlignment(Qt::AlignTop);
    ui->userReviewsFiltersLayout->setAlignment(Qt::AlignTop);

    auto reviewsPaginator = new Pagination(ui->userReviewsResultsPage);
    connect(reviewsPaginator, &Pagination::changedPage, this, &CatalogProductPage::changeUserReviewsPage);
    connect(this, &CatalogProductPage::userReviewsResultsUpdated, reviewsPaginator, &Pagination::changePages);
    ui->userReviewsResultsPageLayout->addWidget(reviewsPaginator,  Qt::AlignHCenter);

    connect(ui->descriptionView->page(), &QWebEnginePage::contentsSizeChanged,
            this, &CatalogProductPage::descriptionViewContentsSizeChanged);
    ui->descriptionView->page()->setBackgroundColor(Qt::transparent);
}

CatalogProductPage::~CatalogProductPage()
{
    if (averageRatingReply != nullptr)
    {
        averageRatingReply->abort();
    }
    if (averageOwnerRatingReply != nullptr)
    {
        averageOwnerRatingReply->abort();
    }
    if (backgroundReply != nullptr)
    {
        backgroundReply->abort();
    }
    for (QNetworkReply *dependentProductReply : std::as_const(dependentProductReplies))
    {
        if (dependentProductReply != nullptr)
        {
            dependentProductReply->abort();
        }
    }
    if (lastReviewsReply != nullptr)
    {
        lastReviewsReply->abort();
    }
    if (logotypeReply != nullptr)
    {
        logotypeReply->abort();
    }
    if (mainReply != nullptr)
    {
        mainReply->abort();
    }
    if (ownedProductsReply != nullptr)
    {
        ownedProductsReply->abort();
    }
    if (pricesReply != nullptr)
    {
        pricesReply->abort();
    }
    if (recommendedSimilarReply != nullptr)
    {
        recommendedSimilarReply->abort();
    }
    if (recommendedPurchasedTogetherReply != nullptr)
    {
        recommendedPurchasedTogetherReply->abort();
    }
    for (QNetworkReply *requiredProductReply : std::as_const(requiredProductReplies))
    {
        if (requiredProductReply != nullptr)
        {
            requiredProductReply->abort();
        }
    }
    if (seriesGamesReply != nullptr)
    {
        seriesGamesReply->abort();
    }
    if (seriesTotalPriceReply != nullptr)
    {
        seriesTotalPriceReply->abort();
    }
    if (wishlistReply != nullptr)
    {
        wishlistReply->abort();
    }
    delete ui;
}

void CatalogProductPage::setApiClient(api::GogApiClient *apiClient)
{
    this->apiClient = apiClient;
}

void initializeSystemRequirements(QWidget *rootWidget, QGridLayout *grid, const api::SupportedOperatingSystem data)
{
    int column = 0;
    for (int i = 0; i < data.systemRequirements.count(); i++)
    {
        QString title;
        if (data.systemRequirements[i].type == "minimum")
        {
            title = "Minimum system requirements";
        }
        else if (data.systemRequirements[i].type == "recommended")
        {
            title = "Recommended system requirements";
        }
        else
        {
            title = "Unknown system requirements";
        }

        grid->addWidget(new QLabel(title, rootWidget), 0, column, 1, i > 0 ? 1 : 2);
        column += grid->columnCount();
    }

    for (int i = 0; i < data.definedRequirements.count(); i++)
    {
        int row = i + 1;
        auto definedRequirementLabel = new QLabel(data.definedRequirements[i].name, rootWidget);
        definedRequirementLabel->setAlignment(Qt::AlignLeft | Qt::AlignTop);
        grid->addWidget(definedRequirementLabel, row, 0);

        for (int j = 0; j < data.systemRequirements.count(); j++)
        {
            QString requirement = data.systemRequirements[j].requirements.value(data.definedRequirements[i].id, "⸺");
            auto requirementLabel = new QLabel(requirement, rootWidget);
            requirementLabel->setWordWrap(true);
            requirementLabel->setAlignment(Qt::AlignLeft | Qt::AlignTop);
            grid->addWidget(requirementLabel, row, j + 1);
        }
    }
}

void CatalogProductPage::initialize(const QVariant &initialData)
{
    id = initialData.toString();
    averageRatingReply = apiClient->getProductAverageRating(id);
    connect(averageRatingReply, &QNetworkReply::finished, this, [this]()
    {
        auto networkReply = averageRatingReply;
        averageRatingReply = nullptr;

        if (networkReply->error() == QNetworkReply::NoError)
        {
            auto resultJson = QJsonDocument::fromJson(QString(networkReply->readAll()).toUtf8()).object();
            api::GetRatingResponse data;
            parseRatingResponse(resultJson, data);

            if (data.count > 0)
            {
                ui->starLabel->setVisible(true);
                ui->ratingLabel->setText(QString::number(data.value, 'g', 2) + "/5");
                ui->ratingLabel->setVisible(true);
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

    averageOwnerRatingReply = apiClient->getProductAverageRating(id, "verified_owner");
    connect(averageOwnerRatingReply, &QNetworkReply::finished, this, [this]()
    {
        auto networkReply = averageOwnerRatingReply;
        averageOwnerRatingReply = nullptr;

        if (networkReply->error() == QNetworkReply::NoError)
        {
            auto resultJson = QJsonDocument::fromJson(QString(networkReply->readAll()).toUtf8()).object();
            api::GetRatingResponse data;
            parseRatingResponse(resultJson, data);

            ui->ownersRatingLabel->setText(QString("Verified owners rating: %1/5").arg(QString::number(data.value, 'g', 2)));
            ui->ownersRatingLabel->setVisible(true);
        }
        else if (networkReply->error() != QNetworkReply::OperationCanceledError)
        {
            qDebug() << networkReply->error()
                     << networkReply->errorString()
                     << QString(networkReply->readAll()).toUtf8();
        }
        networkReply->deleteLater();
    });

    auto countryCode = QLocale::territoryToCode(QLocale::system().territory());
    pricesReply = apiClient->getProductPrices(id, countryCode);
    connect(pricesReply, &QNetworkReply::finished, this, [this]()
    {
        auto networkReply = pricesReply;
        pricesReply = nullptr;

        if (networkReply->error() == QNetworkReply::NoError)
        {
            auto resultJson = QJsonDocument::fromJson(QString(networkReply->readAll()).toUtf8()).object();
            api::GetPricesResponse data;
            parseGetPricesResponse(resultJson, data);
            if (!data.prices.isEmpty())
            {
                auto systemLocale = QLocale::system();
                auto currencyCode = systemLocale.currencySymbol(QLocale::CurrencyIsoCode);
                auto currencySymbol = systemLocale.currencySymbol();
                if (!data.prices.contains(currencyCode))
                {
                    auto usaLocale = QLocale(QLocale::English, QLocale::UnitedStates);
                    currencyCode = usaLocale.currencySymbol(QLocale::CurrencyIsoCode);
                    currencySymbol = usaLocale.currencySymbol();
                }

                auto price = data.prices[currencyCode];
                if (price.basePrice != price.finalPrice)
                {
                    auto discount = round((1.0 - price.finalPrice * 1.0 / price.basePrice) * 100);
                    ui->oldPriceLabel->setText(systemLocale.toCurrencyString(price.basePrice / 100.0));
                    ui->oldPriceLabel->setVisible(true);
                    ui->discountLabel->setText(QString("-%1%").arg(QString::number(discount)));
                    ui->discountLabel->setVisible(true);
                }
                ui->pricelabel->setText(systemLocale.toCurrencyString(price.finalPrice / 100.0, currencySymbol));
                ui->pricelabel->setVisible(true);
            }
        }
        else if (networkReply->error() != QNetworkReply::OperationCanceledError)
        {
            qDebug() << networkReply->error() << networkReply->errorString() << QString(networkReply->readAll()).toUtf8();
        }
        networkReply->deleteLater();
    });

    mainReply = apiClient->getCatalogProductInfo(id, QLocale::system().name(QLocale::TagSeparator::Dash));
    connect(mainReply, &QNetworkReply::finished, this, [this]()
    {
        auto networkReply = mainReply;
        mainReply = nullptr;

        if (networkReply->error() == QNetworkReply::NoError)
        {
            auto resultJson = QJsonDocument::fromJson(QString(networkReply->readAll()).toUtf8()).object();
            api::GetCatalogProductInfoResponse data;
            parseCatalogProductInfoResponse(resultJson, data);

            if (!data.galaxyBackgroundImageLink.isNull())
            {
                backgroundReply = apiClient->getAnything(data.galaxyBackgroundImageLink);
                connect(backgroundReply, &QNetworkReply::finished, this, [this]()
                {
                    auto networkReply = backgroundReply;
                    backgroundReply = nullptr;

                    if (networkReply->error() == QNetworkReply::NoError)
                    {
                        backgroundImage.loadFromData(networkReply->readAll());
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

            if (!data.logoLink.isNull())
            {
                logotypeReply = apiClient->getAnything(data.logoLink);
                connect(logotypeReply, &QNetworkReply::finished, this, [this]()
                {
                    auto networkReply = logotypeReply;
                    logotypeReply = nullptr;

                    if (networkReply->error() == QNetworkReply::NoError)
                    {
                        QPixmap logo;
                        logo.loadFromData(networkReply->readAll());
                        ui->logotypeLabel->setPixmap(logo.scaled(480, 285, Qt::KeepAspectRatioByExpanding));
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

            QLocale systemLocale = QLocale::system();
            if (data.videos.isEmpty() && data.screenshots.isEmpty())
            {
                ui->mediaScrollArea->setVisible(false);
            }
            else
            {
                std::size_t mediaIndex = 0;
                for (const api::ThumbnailedVideo &video : std::as_const(data.videos))
                {
                    // How come it is not displayed on a web-site? Just... why?
                    if (video.provider != "wistia")
                    {
                        auto videoHolder = new VideoHolder(QSize(271, 143), video,apiClient, ui->mediaScrollAreaContents);
                        videoHolder->setCursor(Qt::PointingHandCursor);
                        connect(videoHolder, &VideoHolder::clicked, this, [this, mediaIndex]()
                        {
                            openGalleryOnItem(mediaIndex);
                        });
                        ui->mediaScrollAreaContentsLayout->addWidget(videoHolder);
                        videos.append(video);
                        mediaIndex++;
                    }
                }
                for (const api::FormattedLink &screenshotLink : std::as_const(data.screenshots))
                {
                    auto screenshotHolder = new ImageHolder(QSize(271, 143), screenshotLink,apiClient, ui->mediaScrollAreaContents);
                    screenshotHolder->setCursor(Qt::PointingHandCursor);
                    connect(screenshotHolder, &ImageHolder::clicked, this, [this, mediaIndex]()
                    {
                        openGalleryOnItem(mediaIndex);
                    });
                    ui->mediaScrollAreaContentsLayout->addWidget(screenshotHolder);
                    screenshots.append(screenshotLink);
                    mediaIndex++;
                }
            }

            bool setOS = false;
            QStringList supportedOSInfo(data.supportedOperatingSystems.count());
            for (int i = 0; i < data.supportedOperatingSystems.count(); i++)
            {
                supportedOSInfo[i] = data.supportedOperatingSystems[i].versions;
                if (data.supportedOperatingSystems[i].name == "windows")
                {
                    ui->windowsLabel->setVisible(true);
                    auto tabIndex = ui->systemRequirementsTabWidget->indexOf(ui->windowsTab);
                    ui->systemRequirementsTabWidget->setTabVisible(tabIndex, true);
                    initializeSystemRequirements(
                                ui->windowsTab,
                                ui->windowsTabLayout,
                                data.supportedOperatingSystems[i]);
#ifdef Q_OS_WINDOWS
                    ui->systemRequirementsTabWidget->setCurrentIndex(tabIndex);
                    setOS = true;
#endif
                }
                else if (data.supportedOperatingSystems[i].name == "osx")
                {
                    ui->macOsLabel->setVisible(true);
                    auto tabIndex = ui->systemRequirementsTabWidget->indexOf(ui->macOsTab);
                    ui->systemRequirementsTabWidget->setTabVisible(tabIndex, true);
                    initializeSystemRequirements(
                                ui->macOsTab,
                                ui->macOsTabLayout, data.supportedOperatingSystems[i]);
#ifdef Q_OS_MACOS
                    ui->systemRequirementsTabWidget->setCurrentIndex(tabIndex);
                    setOS = true;
#endif
                }
                else if (data.supportedOperatingSystems[i].name == "linux")
                {
                    ui->linuxLabel->setVisible(true);
                    auto tabIndex = ui->systemRequirementsTabWidget->indexOf(ui->linuxTab);
                    ui->systemRequirementsTabWidget->setTabVisible(tabIndex, true);
                    initializeSystemRequirements(
                                ui->linuxTab,
                                ui->linuxTabLayout, data.supportedOperatingSystems[i]);
#ifdef Q_OS_LINUX
                    ui->systemRequirementsTabWidget->setCurrentIndex(tabIndex);
                    setOS = true;
#endif
                }
            }
            if (!setOS)
            {
                ui->systemRequirementsTabWidget->setCurrentWidget(ui->windowsTab);
            }
            ui->additonalRequirementsLabel->setText(data.additionalRequirements);
            ui->additonalRequirementsLabel->setVisible(!data.additionalRequirements.isEmpty());
            ui->dosboxLabel->setVisible(data.usingDosBox);

            if (data.editions.isEmpty())
            {
                ui->editionsComboBox->setVisible(false);
                if (data.bonuses.isEmpty())
                {
                    ui->goodiesLabel->setVisible(false);
                    ui->goodiesStackWidget->setVisible(false);
                }
                else
                {
                    ui->goodiesLabel->setVisible(true);
                    ui->goodiesStackWidget->setVisible(true);
                    ui->goodiesStackWidget->setCurrentWidget(ui->productGoodiesSection);

                    for (const api::Bonus &bonus : std::as_const(data.bonuses))
                    {
                        ui->productGoodiesSection->layout()->addWidget(
                                    new BonusItem(bonus, ui->productGoodiesSection));
                    }
                }
            }
            else
            {
                ui->editionsComboBox->setVisible(true);

                for (int i = 0; i < data.editions.count(); i++)
                {
                    ui->editionsComboBox->addItem(data.editions[i].name);

                    auto editionCheckbox = new QCheckBox(ui->goodiesComparisonSection);
                    if (data.editions[i].id == data.id)
                    {
                        editionCheckbox->setChecked(true);
                        editionCheckbox->setEnabled(false);
                        ui->editionsComboBox->setCurrentIndex(i);
                    }
                    else
                    {
                        auto editionId = data.editions[i].id;
                        connect(editionCheckbox, &QCheckBox::toggled, this, [this, editionId](bool checked)
                        {
                            if (checked)
                            {
                                emit navigate({Page::CATALOG_PRODUCT, editionId});
                            }
                        });
                    }
                    ui->goodiesComparisonSectionGrid->addWidget(editionCheckbox, 0, i + 1,
                                                                Qt::AlignHCenter | Qt::AlignVCenter);

                    auto editionNameLabel = new QLabel(data.editions[i].name, ui->goodiesComparisonSection);
                    editionNameLabel->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);
                    editionNameLabel->setWordWrap(true);
                    editionNameLabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
                    ui->goodiesComparisonSectionGrid->addWidget(editionNameLabel, 1, i + 1);
                }

                if (data.fullBonusList.isEmpty())
                {
                    ui->goodiesLabel->setVisible(false);
                    ui->goodiesStackWidget->setVisible(false);
                }
                else
                {
                    ui->goodiesLabel->setVisible(true);
                    ui->goodiesStackWidget->setVisible(true);
                    ui->goodiesStackWidget->setCurrentWidget(ui->goodiesComparisonSection);

                    auto goodiesComparisonLabel = new QLabel("Contents", ui->goodiesComparisonSection);
                    goodiesComparisonLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
                    ui->goodiesComparisonSectionGrid->addWidget(goodiesComparisonLabel, 0, 0, 2, 1);
                    for (int i = 0; i < data.fullBonusList.count(); i++)
                    {
                        auto bonusName = data.fullBonusList[i];
                        auto rowIndex = i + 2;
                        auto goodieNameLabel = new QLabel(bonusName, ui->goodiesComparisonSection);
                        goodieNameLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
                        ui->goodiesComparisonSectionGrid->addWidget(goodieNameLabel, rowIndex, 0);
                        for (int j = 0; j < data.editions.count(); j++)
                        {
                            auto isIncluded = data.editions[j].bonusSet.contains(bonusName);
                            auto checkedItem = new CheckedItem(QString(), isIncluded, ui->goodiesComparisonSection);
                            ui->goodiesComparisonSectionGrid->addWidget(checkedItem, rowIndex, j + 1,
                                                                        Qt::AlignHCenter | Qt::AlignVCenter);
                        }
                    }
                }
            }

            ui->titleLabel->setText(data.title);

            ui->libraryButton->setVisible(false);

            QFont tagFont;
            tagFont.setPointSizeF(10.5);
            tagFont.setUnderline(true);
            for (const api::HierarchicalMetaTag &genre : std::as_const(data.tags))
            {
                auto genreLabel = new ClickableLabel(ui->genreHolder);
                genreLabel->setCursor(Qt::PointingHandCursor);
                genreLabel->setText(genre.name);
                genreLabel->setFont(tagFont);
                connect(genreLabel, &ClickableLabel::clicked, this, [this, genreSlug = genre.slug]()
                {
                    emit navigate({ Page::ALL_GAMES, QMap<QString, QVariant>({ std::pair("genre", genreSlug) }) });
                });
                ui->genreHolder->layout()->addWidget(genreLabel);
            }

            for (const api::MetaTag &tag : std::as_const(data.properties))
            {
                auto tagLabel = new ClickableLabel(ui->tagsHolder);
                tagLabel->setCursor(Qt::PointingHandCursor);
                tagLabel->setText(tag.name);
                tagLabel->setFont(tagFont);
                connect(tagLabel, &ClickableLabel::clicked, this, [this, tagSlug = tag.slug]()
                {
                    emit navigate({ Page::ALL_GAMES, QMap<QString, QVariant>({ std::pair("tag", tagSlug) }) });
                });
                ui->tagsHolder->layout()->addWidget(tagLabel);
            }

            ui->tagsLabel->setVisible(data.properties.count() > 0);
            ui->tagsHolder->setVisible(data.properties.count() > 0);

            ui->supportedOSInfoLabel->setText(supportedOSInfo.join(", "));
            if (data.globalReleaseDate.isNull())
            {
                ui->releaseDateLabel->setVisible(false);
                ui->releaseDateInfoLabel->setVisible(false);
                ui->releaseDateInfoLabel->clear();
            }
            else
            {
                ui->releaseDateInfoLabel->setText(systemLocale.toString(data.globalReleaseDate.date()));
                ui->releaseDateLabel->setVisible(true);
                ui->releaseDateInfoLabel->setVisible(true);
            }

            auto developerLabel = new ClickableLabel(ui->infoSidebar);
            developerLabel->setCursor(Qt::PointingHandCursor);
            developerLabel->setFont(tagFont);
            developerLabel->setText(data.developers[0]);
            connect(developerLabel, &ClickableLabel::clicked, this, [this, developerSlug = makeSlugFromText(data.developers[0])]()
            {
                emit navigate({ Page::ALL_GAMES, QMap<QString, QVariant>({ std::pair("developer", developerSlug) }) });
            });
            ui->companyHolder->layout()->addWidget(developerLabel);

            auto publisherLabel = new ClickableLabel(ui->companyHolder);
            publisherLabel->setCursor(Qt::PointingHandCursor);
            publisherLabel->setFont(tagFont);
            publisherLabel->setText(data.publisher);
            connect(publisherLabel, &ClickableLabel::clicked, this, [this, publisherSlug = makeSlugFromText(data.publisher)]()
            {
                emit navigate({ Page::ALL_GAMES, QMap<QString, QVariant>({ std::pair("publisher", publisherSlug) }) });
            });
            ui->companyHolder->layout()->addWidget(publisherLabel);

            if (data.size == 0)
            {
                ui->sizeLabel->setVisible(false);
                ui->sizeInfoLabel->setVisible(false);
                ui->sizeInfoLabel->clear();
            }
            else
            {
                ui->sizeLabel->setVisible(true);
                ui->sizeInfoLabel->setVisible(true);
                ui->sizeInfoLabel->setText(systemLocale.formattedDataSize(static_cast<unsigned long long>(data.size) * 1000 * 1000, 2, QLocale::DataSizeSIFormat));
            }
            if (data.forumLink.isNull())
            {
                ui->linksLabel->setVisible(false);
                ui->linksInfoLabel->setVisible(false);
                ui->linksInfoLabel->clear();
            }
            else
            {
                ui->linksLabel->setVisible(true);
                ui->linksInfoLabel->setVisible(true);
                ui->linksInfoLabel->setText(
                            QString("<a href=\"%1\">GOG store page</a>, <a href=\"%2\">Forum discussions</a>, <a href=\"%3\">Support</a>")
                            .arg(data.storeLink, data.forumLink, data.supportLink));
            }

            QVector<QString> ageRatings = getTerritoryPreferredRatings(systemLocale.territory());

            bool foundRating = false;
            QString ratingDescription;
            QPixmap ratingIcon;
            for (const QString &ratingCode : std::as_const(ageRatings))
            {
                if (data.ratings.contains(ratingCode))
                {
                    foundRating = true;
                    auto contentRating = data.ratings[ratingCode];
                    ratingDescription = QString("%1 Rating: %2%3").arg(
                                ratingCode,
                                contentRating.category.isEmpty() ? QString::number(contentRating.ageRating) + "+" : contentRating.category,
                                !contentRating.contentDescriptors.empty()
                                    ? " (" + contentRating.contentDescriptors.join(", ") + ")"
                                    : "");
                    if (ratingCode == "PEGI" || ratingCode == "USK" || ratingCode == "BR")
                    {
                        ratingIcon = QPixmap(QString(":/icons/%1_%2.svg").arg(ratingCode).arg(contentRating.ageRating));
                    }
                    else if (ratingCode == "ESRB")
                    {
                        switch (contentRating.categoryId)
                        {
                        case 15:
                            ratingIcon = QPixmap(QString(":/icons/ESRB_2013_Everyone.svg"));
                            break;
                        case 16:
                            ratingIcon = QPixmap(QString(":/icons/ESRB_2013_Everyone_10+.svg"));
                            break;
                        case 17:
                            ratingIcon = QPixmap(QString(":/icons/ESRB_2013_Teen.svg"));
                            break;
                        case 18:
                            ratingIcon = QPixmap(QString(":/icons/ESRB_2013_Mature.svg"));
                            break;
                        case 19:
                            ratingIcon = QPixmap(QString(":/icons/ESRB_2013_Adults_Only_18+.svg"));
                            break;
                        }
                    }
                    break;
                }
            }
            ui->contentRatingLabel->setVisible(foundRating);
            ui->contentRatingInfoLabel->setText(ratingDescription);
            ui->contentRatingInfoLabel->setVisible(!ratingDescription.isNull());
            ui->contentRatingIconLabel->setVisible(!ratingIcon.isNull());
            ui->contentRatingIconLabel->setPixmap(ratingIcon);

            for (const api::MetaTag &feature : std::as_const(data.features))
            {
                auto featureItem = new FeatureItem(feature, ui->gameDetails);
                featureItem->setCursor(Qt::PointingHandCursor);
                connect(featureItem, &FeatureItem::clicked, this, [this, featureSlug = feature.slug]()
                {
                    emit navigate({ Page::ALL_GAMES, QMap<QString, QVariant>({ std::pair("feature", featureSlug) }) });
                });
                ui->gameFeaturesLayout->addWidget(featureItem);
            }

            QString countryCode = QLocale::territoryToCode(systemLocale.territory());
            QString currencyCode = systemLocale.currencySymbol(QLocale::CurrencyIsoCode);
            QString systemLanguage = QLocale::languageToCode(systemLocale.language(), QLocale::ISO639Part1);
            bool foundPreferredLanguage = false;
            for (int i = 0; i < data.localizations.count(); i++)
            {
                auto languageLabel = new QLabel(data.localizations[i].language.name, ui->gameDetails);
                foundPreferredLanguage = foundPreferredLanguage || data.localizations[i].language.code == systemLanguage;
                languageLabel->setStyleSheet("font-size: 11pt;");
                languageLabel->setWordWrap(true);
                ui->languagesLayout->addWidget(languageLabel, i, 0);
                ui->languagesLayout->addWidget(new CheckedItem("audio", data.localizations[i].localizedAuio, ui->gameDetails), i, 1);
                ui->languagesLayout->addWidget(new CheckedItem("text", data.localizations[i].localizedText, ui->gameDetails), i, 2);
            }
            QString firstLanguage = foundPreferredLanguage ? systemLocale.nativeLanguageName() :"English";
            if (data.localizations.count() > 1)
            {
                ui->supportedLanguagesLabel->setText(QString("%1 & %2 others").arg(firstLanguage,
                                                                          QString::number(data.localizations.count() - 1)));
            }
            else
            {
                ui->supportedLanguagesLabel->setText(firstLanguage);
            }

            ui->mainGameLabel->setVisible(false);
            requiredProducts.resize(data.requiresGames.count());
            requiredProductsLeft = data.requiresGames.count();
            requiredProductReplies.resize(requiredProductsLeft);
            for (std::size_t mainPartIndex = 0; mainPartIndex < data.requiresGames.count(); mainPartIndex++)
            {
                requiredProductReplies[mainPartIndex] = apiClient->getAnything(data.requiresGames[mainPartIndex]);
                connect(requiredProductReplies[mainPartIndex], &QNetworkReply::finished,
                        this, [this, mainPartIndex]()
                {
                    auto networkReply = requiredProductReplies[mainPartIndex];
                    requiredProductReplies[mainPartIndex] = nullptr;
                    requiredProductsLeft--;

                    if (networkReply->error() == QNetworkReply::NoError)
                    {
                        auto resultJson = QJsonDocument::fromJson(QString(networkReply->readAll()).toUtf8()).object();
                        parseCatalogProductInfoResponse(resultJson, requiredProducts[mainPartIndex]);
                    }
                    else if (networkReply->error() != QNetworkReply::OperationCanceledError)
                    {
                        qDebug() << networkReply->error()
                                 << networkReply->errorString()
                                 << QString(networkReply->readAll()).toUtf8();
                    }
                    networkReply->deleteLater();

                    if (requiredProductsLeft == 0)
                    {
                        for (const api::GetCatalogProductInfoResponse &mainPart : std::as_const(requiredProducts))
                        {
                            if (!mainPart.title.isNull())
                            {
                                auto mainPartItem = new DependentProductItem(mainPart, apiClient, ui->mainContentFrame);
                                mainPartItem->setCursor(Qt::PointingHandCursor);
                                connect(mainPartItem, &DependentProductItem::clicked,
                                        this, [this, productId = mainPart.id]()
                                {
                                    emit navigate({ Page::CATALOG_PRODUCT, productId });
                                });
                                ui->mainGameLayout->addWidget(mainPartItem);
                            }
                        }
                        ui->mainGameLabel->setVisible(ui->mainGameLayout->count() > 0);
                    }
                });
            }

            ui->dlcsLabel->setVisible(false);
            dependentProducts.resize(data.requiredByGames.count());
            dependentProductsLeft = data.requiredByGames.count();
            dependentProductReplies.resize(dependentProductsLeft);
            for (std::size_t dlcIndex = 0; dlcIndex < data.requiredByGames.count(); dlcIndex++)
            {
                dependentProductReplies[dlcIndex] = apiClient->getAnything(data.requiredByGames[dlcIndex]);
                connect(dependentProductReplies[dlcIndex], &QNetworkReply::finished,
                        this, [this, dlcIndex]()
                {
                    auto networkReply = dependentProductReplies[dlcIndex];
                    dependentProductReplies[dlcIndex] = nullptr;
                    dependentProductsLeft--;

                    if (networkReply->error() == QNetworkReply::NoError)
                    {
                        auto resultJson = QJsonDocument::fromJson(QString(networkReply->readAll()).toUtf8()).object();
                        parseCatalogProductInfoResponse(resultJson, dependentProducts[dlcIndex]);
                    }
                    else if (networkReply->error() != QNetworkReply::OperationCanceledError)
                    {
                        qDebug() << networkReply->error()
                                 << networkReply->errorString()
                                 << QString(networkReply->readAll()).toUtf8();
                    }
                    networkReply->deleteLater();

                    if (dependentProductsLeft == 0)
                    {
                        for (const api::GetCatalogProductInfoResponse &dlc : std::as_const(dependentProducts))
                        {
                            if (!dlc.title.isNull())
                            {
                                auto dlcItem = new DependentProductItem(dlc, apiClient, ui->mainContentFrame);
                                dlcItem->setCursor(Qt::PointingHandCursor);
                                connect(dlcItem, &DependentProductItem::clicked,
                                        this, [this, productId = dlc.id]()
                                {
                                    emit navigate({ Page::CATALOG_PRODUCT, productId });
                                });
                                ui->dlcsLayout->addWidget(dlcItem);
                            }
                        }
                        ui->dlcsLabel->setVisible(ui->dlcsLayout->count() > 0);
                    }
                });
            }

            if (data.series.name.isNull())
            {
                ui->seriesLabel->setVisible(false);
                ui->seriesBuyButton->setVisible(false);
                ui->seriesStackedWidget->setVisible(false);
            }
            else
            {
                seriesGamesReply = apiClient->getSeriesGames(data.series.id);
                connect(seriesGamesReply, &QNetworkReply::finished, this, [this]()
                {
                    auto networkReply = seriesGamesReply;
                    seriesGamesReply = nullptr;

                    if (networkReply->error() == QNetworkReply::NoError)
                    {
                        auto resultJson = QJsonDocument::fromJson(QString(networkReply->readAll()).toUtf8()).object();
                        api::GetSeriesGamesResponse data;
                        parseGetSeriesGamesResponse(resultJson, data);

                        unsigned int count = 0;
                        for (const api::SeriesGame &item : std::as_const(data.items))
                        {
                            if (item.visibleInCatalog)
                            {
                                count++;
                                auto productItem = new SimpleProductItem(ui->seriesResultPage);
                                productItem->setCover(item.imageLink, apiClient);
                                productItem->setTitle(item.title);

                                connect(this, &CatalogProductPage::ownedProductsChanged,
                                        productItem, [productItem, productId = item.id](const QSet<const QString> &ids)
                                {
                                    productItem->setOwned(ids.contains(productId));
                                });
                                productItem->setOwned(ownedProducts.contains(item.id));
                                connect(this, &CatalogProductPage::wishlistChanged,
                                        productItem, [productItem, productId = item.id](const QSet<const QString> &ids)
                                {
                                    productItem->setWishlisted(ids.contains(productId));
                                });
                                productItem->setWishlisted(wishlist.contains(item.id));
                                connect(apiClient, &api::GogApiClient::authenticated,
                                        productItem, &SimpleProductItem::switchUiAuthenticatedState);
                                productItem->switchUiAuthenticatedState(apiClient->isAuthenticated());

                                connect(productItem, &SimpleProductItem::clicked,
                                        this, [this, productId = item.id]()
                                {
                                    emit navigate({Page::CATALOG_PRODUCT, productId});
                                });
                                ui->seriesResultPage->layout()->addWidget(productItem);
                            }
                        }

                        ui->seriesLabel->setText(QString("%1 (%2)")
                                                 .arg(ui->seriesLabel->text())
                                                 .arg(count));
                        ui->seriesStackedWidget->setCurrentWidget(ui->seriesResultPage);
                    }
                    else if (networkReply->error() != QNetworkReply::OperationCanceledError)
                    {
                        qDebug() << networkReply->error()
                                 << networkReply->errorString()
                                 << QString(networkReply->readAll()).toUtf8();
                    }
                    networkReply->deleteLater();
                });

                seriesTotalPriceReply = apiClient->getSeriesPrices(data.series.id, countryCode, currencyCode);
                connect(seriesTotalPriceReply, &QNetworkReply::finished, this, [this]()
                {
                    auto networkReply = seriesTotalPriceReply;
                    seriesTotalPriceReply = nullptr;

                    if (networkReply->error() == QNetworkReply::NoError)
                    {
                        auto resultJson = QJsonDocument::fromJson(QString(networkReply->readAll()).toUtf8()).object();
                        api::GetPricesResponse data;
                        parseGetPricesResponse(resultJson, data);

                        if (!data.currencies.isEmpty())
                        {
                            QLocale systemLocale = QLocale::system();
                            QString preferredCurrency = data.currencies[0];
                            auto preferredPrice = data.prices[preferredCurrency];
                            ui->seriesBuyButton->setEnabled(apiClient->isAuthenticated());
                            ui->seriesBuyButton->setText(QString("Buy whole series for %1")
                                                         .arg(systemLocale.toCurrencyString(preferredPrice.finalPrice / 100., preferredCurrency)));
                            ui->seriesBuyButton->setVisible(true);
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
            }
            recommendedPurchasedTogetherReply = apiClient->getProductRecommendationsPurchasedTogether(id, countryCode, currencyCode, 8);
            connect(recommendedPurchasedTogetherReply, &QNetworkReply::finished, this, [this]()
            {
                auto networkReply = recommendedPurchasedTogetherReply;
                recommendedPurchasedTogetherReply = nullptr;

                if (networkReply->error() == QNetworkReply::NoError)
                {
                    auto resultJson = QJsonDocument::fromJson(QString(networkReply->readAll()).toUtf8()).object();
                    api::GetRecommendationsResponse data;
                    parseRecommendationsResponse(resultJson, data);

                    for (const api::Recommendation &recommendation : std::as_const(data.products))
                    {
                        auto recommendationItem = new SimpleProductItem(ui->purchasedTogetherResultPage);
                        recommendationItem->setCover(recommendation.details.imageHorizontalUrl, apiClient);
                        recommendationItem->setTitle(recommendation.details.title);
                        if (recommendation.pricing.priceSet)
                        {
                            unsigned char discount = round(1. * (recommendation.pricing.basePrice - recommendation.pricing.finalPrice) / recommendation.pricing.basePrice * 100);
                            recommendationItem->setPrice(recommendation.pricing.basePrice / 100.,
                                                         recommendation.pricing.finalPrice / 100.,
                                                         discount,
                                                         recommendation.pricing.basePrice == 0,
                                                         recommendation.pricing.currency);
                        }

                        connect(this, &CatalogProductPage::ownedProductsChanged,
                                recommendationItem, [recommendationItem, productId = recommendation.productId](const QSet<const QString> &ids)
                        {
                            recommendationItem->setOwned(ids.contains(productId));
                        });
                        recommendationItem->setOwned(ownedProducts.contains(recommendation.productId));
                        connect(this, &CatalogProductPage::wishlistChanged,
                                recommendationItem, [recommendationItem, productId = recommendation.productId](const QSet<const QString> &ids)
                        {
                            recommendationItem->setWishlisted(ids.contains(productId));
                        });
                        recommendationItem->setWishlisted(wishlist.contains(recommendation.productId));
                        connect(apiClient, &api::GogApiClient::authenticated,
                                recommendationItem, &SimpleProductItem::switchUiAuthenticatedState);
                        recommendationItem->switchUiAuthenticatedState(apiClient->isAuthenticated());

                        connect(recommendationItem, &SimpleProductItem::clicked,
                                this, [this, productId = recommendation.productId]()
                        {
                            emit navigate({Page::CATALOG_PRODUCT, productId});
                        });
                        ui->purchasedTogetherResultPage->layout()->addWidget(recommendationItem);
                    }
                    ui->purchasedTogetherStackedWidget->setCurrentWidget(ui->purchasedTogetherResultPage);
                }
                else if (networkReply->error() != QNetworkReply::OperationCanceledError)
                {
                    ui->purchasedTogetherLabel->setVisible(false);
                    ui->purchasedTogetherStackedWidget->setVisible(false);
                }
                networkReply->deleteLater();
            });
            recommendedSimilarReply = apiClient->getProductRecommendationsSimilar(id, countryCode, currencyCode, 8);
            connect(recommendedSimilarReply, &QNetworkReply::finished, this, [this]()
            {
                auto networkReply = recommendedSimilarReply;
                recommendedSimilarReply = nullptr;

                if (networkReply->error() == QNetworkReply::NoError)
                {
                    auto resultJson = QJsonDocument::fromJson(QString(networkReply->readAll()).toUtf8()).object();
                    api::GetRecommendationsResponse data;
                    parseRecommendationsResponse(resultJson, data);

                    for (const api::Recommendation &recommendation : std::as_const(data.products))
                    {
                        auto recommendationItem = new SimpleProductItem(ui->similarProductsResultPage);
                        recommendationItem->setCover(recommendation.details.imageHorizontalUrl, apiClient);
                        recommendationItem->setTitle(recommendation.details.title);
                        if (recommendation.pricing.priceSet)
                        {
                            unsigned char discount = round(1. * (recommendation.pricing.basePrice - recommendation.pricing.finalPrice) / recommendation.pricing.basePrice * 100);
                            recommendationItem->setPrice(recommendation.pricing.basePrice / 100.,
                                                         recommendation.pricing.finalPrice / 100.,
                                                         discount,
                                                         recommendation.pricing.basePrice == 0,
                                                         recommendation.pricing.currency);
                        }

                        connect(this, &CatalogProductPage::ownedProductsChanged,
                                recommendationItem, [recommendationItem, productId = recommendation.productId](const QSet<const QString> &ids)
                        {
                            recommendationItem->setOwned(ids.contains(productId));
                        });
                        recommendationItem->setOwned(ownedProducts.contains(recommendation.productId));
                        connect(this, &CatalogProductPage::wishlistChanged,
                                recommendationItem, [recommendationItem, productId = recommendation.productId](const QSet<const QString> &ids)
                        {
                            recommendationItem->setWishlisted(ids.contains(productId));
                        });
                        recommendationItem->setWishlisted(wishlist.contains(recommendation.productId));
                        connect(apiClient, &api::GogApiClient::authenticated,
                                recommendationItem, &SimpleProductItem::switchUiAuthenticatedState);
                        recommendationItem->switchUiAuthenticatedState(apiClient->isAuthenticated());

                        connect(recommendationItem, &SimpleProductItem::clicked,
                                this, [this, productId = recommendation.productId]()
                        {
                            emit navigate({Page::CATALOG_PRODUCT, productId});
                        });
                        ui->similarProductsResultPage->layout()->addWidget(recommendationItem);
                    }
                    ui->similarProductsStackedWidget->setCurrentWidget(ui->similarProductsResultPage);
                }
                else if (networkReply->error() != QNetworkReply::OperationCanceledError)
                {
                    ui->similarProductsLabel->setVisible(false);
                    ui->similarProductsStackedWidget->setVisible(false);
                }
                networkReply->deleteLater();
            });

            ui->descriptionView->setHtml("<style>body{max-width:100%;}img{max-width:100%;}</style>" + data.description);
            ui->featuresLabel->setText(data.featuresDescription);
            ui->featuresLabel->setVisible(!data.featuresDescription.isNull());
            ui->copyrightsLabel->setText(data.copyrights);
            ui->copyrightsLabel->setVisible(!data.copyrights.isNull());

            initializeUserReviewsFilters();
            updateUserReviews();

            // Should it be controlled by user's territory age rating system instead?
            if (data.ratings.contains("GOG") && data.ratings["GOG"].ageRating >= 18)
            {
                ui->contentStack->setCurrentWidget(ui->contentRatingWarningPage);
            }
            else
            {
                ui->contentStack->setCurrentWidget(ui->mainPage);
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
}

void CatalogProductPage::switchUiAuthenticatedState(bool authenticated)
{
    StoreBasePage::switchUiAuthenticatedState(authenticated);
    ui->cartButton->setEnabled(authenticated);
    ui->seriesBuyButton->setEnabled(authenticated);
    ui->wishlistButton->setEnabled(authenticated);
    ui->wishlistButton->setVisible(true);

    if (ownedProductsReply != nullptr)
    {
        ownedProductsReply->abort();
    }
    if (wishlistReply != nullptr)
    {
        wishlistReply->abort();
    }
    if (authenticated)
    {
        ownedProductsReply = apiClient->getOwnedLicensesIds();
        connect(ownedProductsReply, &QNetworkReply::finished, this, [this]()
        {
            auto networkReply = ownedProductsReply;
            ownedProductsReply = nullptr;
            if (networkReply->error() == QNetworkReply::NoError)
            {
                auto resultJson = QJsonDocument::fromJson(QString(networkReply->readAll()).toUtf8());
                auto ownedProducts = resultJson.toVariant().toList();
                for (const QVariant &id : std::as_const(ownedProducts))
                {
                    this->ownedProducts.insert(id.toString());
                }
                if (this->ownedProducts.contains(id))
                {
                    ui->cartButton->setIcon(QIcon(":icons/gift.svg"));
                    ui->cartButton->setStyleSheet("QPushButton { border: 1px solid #648600; background: transparent; color: #648600; padding: 15px; } QPushButton:hover { background-color: qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:1, stop:0 rgb(180, 230, 60), stop:1 rgb(140, 202, 39)); border-color: #96bd27; color: white; }");
                    ui->cartButton->setText("Buy as gift");
                    ui->libraryButton->setVisible(true);
                    ui->wishlistButton->setVisible(false);
                }
                emit ownedProductsChanged(this->ownedProducts);
            }
            else if (networkReply->error() != QNetworkReply::OperationCanceledError)
            {
                qDebug() << networkReply->error()
                         << networkReply->errorString()
                         << QString(networkReply->readAll()).toUtf8();
            }

            networkReply->deleteLater();
        });
        wishlistReply = apiClient->getWishlistIds();
        connect(wishlistReply, &QNetworkReply::finished, this, [this]()
        {
            auto networkReply = wishlistReply;
            wishlistReply = nullptr;
            if (networkReply->error() == QNetworkReply::NoError)
            {
                auto resultJson = QJsonDocument::fromJson(QString(networkReply->readAll()).toUtf8());
                auto wishlistedItems = resultJson["wishlist"].toObject();
                for (const QString &key : wishlistedItems.keys())
                {
                    if (wishlistedItems[key].toBool())
                    {
                        wishlist.insert(key);
                    }
                }
                ui->wishlistButton->setChecked(wishlist.contains(id));
                emit wishlistChanged(wishlist);
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
    else
    {
        ui->cartButton->setStyleSheet("background-color: qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:1, stop:0 rgb(180, 230, 60), stop:1 rgb(140, 202, 39)); color: white; padding: 15px;");
        ui->cartButton->setText("Add to cart");
        ui->wishlistButton->setChecked(false);
        ui->libraryButton->setVisible(false);
        ownedProducts.clear();
        emit ownedProductsChanged(ownedProducts);
        wishlist.clear();
        emit wishlistChanged(wishlist);
    }
}

void CatalogProductPage::on_acceptContentWarningButton_clicked()
{
    ui->contentStack->setCurrentWidget(ui->mainPage);
}

void CatalogProductPage::on_goBackButton_clicked()
{
    emit navigateBack();
}

void CatalogProductPage::descriptionViewContentsSizeChanged(const QSizeF &size)
{
    ui->descriptionView->setFixedHeight(size.height());
}

void CatalogProductPage::initializeUserReviewsFilters()
{
    ui->userReviewsFiltersLayout->addSpacing(44);
    QLayout *filterAreaLayout;
    auto reviewsLanguageCollapsibleArea = new CollapsibleArea("Written in", ui->mainContentFrame);
    filterAreaLayout = new QVBoxLayout();
    filterAreaLayout->setAlignment(Qt::AlignTop);
    for (const QString &language : std::as_const(reviewFilters.allLanguages))
    {
        QLocale languageLocale(QString(language).replace('-', '_'));
        auto reviewsLanguageCheckbox = new QCheckBox(languageLocale.nativeLanguageName(), reviewsLanguageCollapsibleArea);
        connect(reviewsLanguageCheckbox, &QCheckBox::clicked, this, [this, language](bool checked)
        {
            if (checked)
            {
                reviewFilters.languages.insert(language);
            }
            else
            {
                reviewFilters.languages.remove(language);
            }
            reviewsPage = 1;
            updateUserReviews();
        });
        filterAreaLayout->addWidget(reviewsLanguageCheckbox);
    }
    auto reviewsOtherLanguageCheckbox = new QCheckBox("Others", reviewsLanguageCollapsibleArea);
    connect(reviewsOtherLanguageCheckbox, &QCheckBox::clicked, this, [this](bool checked)
    {
       reviewFilters.otherLanguages = checked;
       reviewsPage = 1;
       updateUserReviews();
    });
    filterAreaLayout->addWidget(reviewsOtherLanguageCheckbox);
    reviewsLanguageCollapsibleArea->setContentLayout(filterAreaLayout);
    ui->userReviewsFiltersLayout->addWidget(reviewsLanguageCollapsibleArea);

    auto reviewsAuthorCollapsibleArea = new CollapsibleArea("Written by", ui->mainContentFrame);
    filterAreaLayout = new QVBoxLayout();
    filterAreaLayout->setAlignment(Qt::AlignTop);
    auto reviewedByOwnerButton = new QRadioButton("Verified owners", reviewsAuthorCollapsibleArea);
    connect(reviewedByOwnerButton, &QRadioButton::clicked, this, [this](bool checked)
    {
        if (checked)
        {
            reviewFilters.reviewedByOwner = true;
            reviewsPage = 1;
            updateUserReviews();
        }
    });
    filterAreaLayout->addWidget(reviewedByOwnerButton);
    auto reviewedByOthersButton = new QRadioButton("Others", reviewsAuthorCollapsibleArea);
    connect(reviewedByOthersButton, &QRadioButton::clicked, this, [this](bool checked)
    {
        if (checked)
        {
            reviewFilters.reviewedByOwner = false;
            reviewsPage = 1;
            updateUserReviews();
        }
    });
    filterAreaLayout->addWidget(reviewedByOthersButton);
    reviewsAuthorCollapsibleArea->setContentLayout(filterAreaLayout);
    ui->userReviewsFiltersLayout->addWidget(reviewsAuthorCollapsibleArea);

    auto reviewsAddedCollapsibleArea = new CollapsibleArea("Added", ui->mainContentFrame);
    filterAreaLayout = new QVBoxLayout();
    filterAreaLayout->setAlignment(Qt::AlignTop);

    auto reviewsAddedLastDaysGroup = new QButtonGroup(reviewsAddedCollapsibleArea);
    auto reviewsAddedLastMonthButton = new QRadioButton("Last 30 days", reviewsAddedCollapsibleArea);
    reviewsAddedLastDaysGroup->addButton(reviewsAddedLastMonthButton, 1);
    filterAreaLayout->addWidget(reviewsAddedLastMonthButton);
    auto reviewsAddedLast3MonthsButton = new QRadioButton("Last 90 days", reviewsAddedCollapsibleArea);
    reviewsAddedLastDaysGroup->addButton(reviewsAddedLast3MonthsButton, 2);
    filterAreaLayout->addWidget(reviewsAddedLast3MonthsButton);
    auto reviewsAddedLast6MonthsButton = new QRadioButton("Last 6 months", reviewsAddedCollapsibleArea);
    reviewsAddedLastDaysGroup->addButton(reviewsAddedLast6MonthsButton, 3);
    filterAreaLayout->addWidget(reviewsAddedLast6MonthsButton);
    auto reviewsAddedWheneverButton = new QRadioButton("Whenever", reviewsAddedCollapsibleArea);
    reviewsAddedLastDaysGroup->addButton(reviewsAddedWheneverButton, 4);
    filterAreaLayout->addWidget(reviewsAddedWheneverButton);
    connect(reviewsAddedLastDaysGroup, &QButtonGroup::idToggled, this, [this](int id, bool checked)
    {
        if (checked)
        {
            switch (id)
            {
            case 1:
            case 2:
            case 3:
                reviewFilters.lastDays = id * 30;
            case 4:
                reviewFilters.lastDays = std::nullopt;
            }

            reviewsPage = 1;
            updateUserReviews();
        }
    });

    auto line = new QFrame(reviewsAddedCollapsibleArea);
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);
    filterAreaLayout->addWidget(line);

    auto reviewsAddedVersionGroup = new QButtonGroup(reviewsAddedCollapsibleArea);
    auto reviewedReleaseVersionButton = new QRadioButton("After release", reviewsAddedCollapsibleArea);
    reviewsAddedVersionGroup->addButton(reviewedReleaseVersionButton, 1);
    filterAreaLayout->addWidget(reviewedReleaseVersionButton);
    auto reviewedDevVersionButton = new QRadioButton("During Early Access", reviewsAddedCollapsibleArea);
    reviewsAddedVersionGroup->addButton(reviewedDevVersionButton, 2);
    filterAreaLayout->addWidget(reviewedDevVersionButton);
    connect(reviewsAddedVersionGroup, &QButtonGroup::idToggled, this, [this](int id, bool checked)
    {
        if (checked)
        {
            reviewFilters.reviewedDuringDevelopment = id == 2;
            reviewsPage = 1;
            updateUserReviews();
        }
    });

    reviewsAddedCollapsibleArea->setContentLayout(filterAreaLayout);
    ui->userReviewsFiltersLayout->addWidget(reviewsAddedCollapsibleArea);
}

void CatalogProductPage::updateUserReviews()
{
    ui->userReviewsStackedWidget->setCurrentWidget(ui->userReviewsLoaderPage);
    if (lastReviewsReply != nullptr)
    {
        lastReviewsReply->abort();
    }
    while (!ui->userReviewsContentsLayout->isEmpty())
    {
        auto item = ui->userReviewsContentsLayout->itemAt(0);
        ui->userReviewsContentsLayout->removeItem(item);
        item->widget()->deleteLater();
        delete item;
    }

    lastReviewsReply = apiClient->getProductReviews(id, reviewFilters, reviewsOrder, reviewsPageSize, reviewsPage);
    connect(lastReviewsReply, &QNetworkReply::finished, this, [this]()
    {
        auto networkReply = lastReviewsReply;
        lastReviewsReply = nullptr;

        if (networkReply->error() == QNetworkReply::NoError)
        {
            auto resultJson = QJsonDocument::fromJson(QString(networkReply->readAll()).toUtf8()).object();
            api::GetReviewsResponse data;
            parseReviewsResponse(resultJson, data);

            ui->userReviewsLabel->setVisible(data.reviewable);
            ui->userReviewsHeader->setVisible(data.reviewable);
            ui->overallRatingLabel->setText(QString("Overall rating: %1/5").arg(QString::number(data.overallAvgRating, 'g', 2)));
            if (data.items.isEmpty())
            {
                ui->filteredRatingLabel->setVisible(false);
                ui->userReviewsStackedWidget->setCurrentWidget(ui->userReviewsNotFoundPage);
            }
            else
            {
                ui->filteredRatingLabel->setText(QString("Filters based rating: %1/5").arg(QString::number(data.filteredAvgRating, 'g', 2)));
                ui->filteredRatingLabel->setVisible(true);
                for (const api::Review &review : std::as_const(data.items))
                {
                    auto reviewItem = new ReviewItem(review, review.id == data.mostHelpfulReviewId,
                                                     apiClient,
                                                     ui->userReviewsResultsPage);
                    ui->userReviewsContentsLayout->addWidget(reviewItem);
                }
                ui->userReviewsStackedWidget->setCurrentWidget(ui->userReviewsResultsPage);
            }
            emit userReviewsResultsUpdated(data.page, data.pages);
            ui->userReviewsStackedWidget->setVisible(data.reviewable);
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

void CatalogProductPage::changeUserReviewsPage(unsigned short page)
{
    reviewsPage = page;
    updateUserReviews();
}

void CatalogProductPage::on_userReviewsPageSizeComboBox_currentIndexChanged(int index)
{
    switch (index)
    {
    case 0:
        reviewsPageSize = 5;
        break;
    case 1:
        reviewsPageSize = 15;
        break;
    case 2:
        reviewsPageSize = 30;
        break;
    case 3:
        reviewsPageSize = 60;
        break;
    }
    reviewsPage = 0;
    updateUserReviews();
}

void CatalogProductPage::on_userReviewsSortOrderComboBox_currentIndexChanged(int index)
{
    switch (index)
    {
    case 0:
        reviewsOrder = {"votes", false};
        break;
    case 1:
        reviewsOrder = {"rating", false};
        break;
    case 2:
        reviewsOrder = {"rating", true};
        break;
    case 3:
        reviewsOrder = {"date", false};
        break;
    }
    reviewsPage = 0;
    updateUserReviews();
}

void CatalogProductPage::paintEvent(QPaintEvent *event)
{
    if (!backgroundImage.isNull())
    {
        QPainter backgroundPainter(this);
        int selfMiddle = width() / 2;
        int backgroundMiddle = backgroundImage.width() / 2;
        backgroundPainter.drawPixmap(0, 0, backgroundImage,
                                     std::max(0, backgroundMiddle - selfMiddle), 0,
                                     std::min(width(), backgroundImage.width()), std::min(height(), backgroundImage.height()));
    }

    QWidget::paintEvent(event);
}

void CatalogProductPage::openGalleryOnItem(std::size_t index)
{
    CatalogProductMediaDialog dialog(videos, screenshots, apiClient, this);
    dialog.viewMedia(index);
    dialog.exec();
}

void CatalogProductPage::on_wishlistButton_clicked()
{

}


void CatalogProductPage::on_wishlistButton_toggled(bool checked)
{
    if (checked)
    {
        ui->wishlistButton->setText("Wishlisted");
    }
    else
    {
        ui->wishlistButton->setText("Wishlist it");
    }
}


void CatalogProductPage::on_libraryButton_clicked()
{
    emit navigate({ Page::OWNED_PRODUCT, id });
}

