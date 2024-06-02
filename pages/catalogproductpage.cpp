#include "catalogproductpage.h"
#include "ui_catalogproductpage.h"

#include <cmath>

#include <QCheckBox>
#include <QJsonDocument>
#include <QLocale>
#include <QNetworkReply>
#include <QScrollBar>

#include "../api/utils/catalogproductserialization.h"
#include "../api/utils/priceserialization.h"
#include "../api/utils/recommendationserialization.h"
#include "../api/utils/reviewserialization.h"
#include "../api/utils/seriesgameserialization.h"

#include "../layouts/flowlayout.h"
#include "../widgets/bonusitem.h"
#include "../widgets/checkeditem.h"
#include "../widgets/featureitem.h"
#include "../widgets/imageholder.h"
#include "../widgets/pagination.h"
#include "../widgets/reviewitem.h"
#include "../widgets/simpleproductitem.h"
#include "../widgets/videoholder.h"

CatalogProductPage::CatalogProductPage(QWidget *parent) :
    BasePage(parent),
    averageRatingReply(nullptr),
    averageOwnerRatingReply(nullptr),
    lastReviewsReply(nullptr),
    mainReply(nullptr),
    pricesReply(nullptr),
    recommendedPurchasedTogetherReply(nullptr),
    recommendedSimilarReply(nullptr),
    seriesGamesReply(nullptr),
    seriesTotalPriceReply(nullptr),
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
    ui->gameDetailsLayout->setAlignment(Qt::AlignTop);
    ui->gameFeaturesLayout->setAlignment(Qt::AlignTop);
    ui->languagesLayout->setAlignment(Qt::AlignTop);

    auto productGoodiesSectionLayout = new FlowLayout(ui->productGoodiesSection, -1, 15, 12);
    ui->productGoodiesSection->setLayout(productGoodiesSectionLayout);

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
    reviewsPage = 1;
    reviewsPageSize = 5;
    reviewsOrder = {"votes", false};
    auto reviewsPaginator = new Pagination(ui->userReviewsResultsPage);
    connect(reviewsPaginator, &Pagination::changedPage, this, &CatalogProductPage::changeUserReviewsPage);
    connect(this, &CatalogProductPage::userReviewsResultsUpdated, reviewsPaginator, &Pagination::changePages);
    ui->userReviewsResultsPageLayout->addWidget(reviewsPaginator, 2, 0,  Qt::AlignHCenter);

    connect(ui->descriptionView->page(), &QWebEnginePage::contentsSizeChanged,
            this, &CatalogProductPage::descriptionViewContentsSizeChanged);
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
    if (lastReviewsReply != nullptr)
    {
        lastReviewsReply->abort();
    }
    if (mainReply != nullptr)
    {
        mainReply->abort();
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
    if (seriesGamesReply != nullptr)
    {
        seriesGamesReply->abort();
    }
    if (seriesTotalPriceReply != nullptr)
    {
        seriesTotalPriceReply->abort();
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
    id = initialData.toULongLong();
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

    updateUserReviews();

    mainReply = apiClient->getCatalogProductInfo(id, QLocale::languageToCode(QLocale::system().language(), QLocale::ISO639Part1));
    connect(mainReply, &QNetworkReply::finished, this, [this]()
    {
        auto networkReply = mainReply;
        mainReply = nullptr;

        if (networkReply->error() == QNetworkReply::NoError)
        {
            auto resultJson = QJsonDocument::fromJson(QString(networkReply->readAll()).toUtf8()).object();
            api::GetCatalogProductInfoResponse data;
            parseCatalogProductInfoResponse(resultJson, data);

            if (data.videos.isEmpty() && data.screenshots.isEmpty())
            {
                ui->mediaScrollArea->setVisible(false);
            }
            else
            {
                for (const api::ThumbnailedVideo &video : std::as_const(data.videos))
                {
                    auto videoHolder = new VideoHolder(QSize(271, 152), video,apiClient, ui->mediaScrollAreaContents);
                    ui->mediaScrollAreaContentsLayout->addWidget(videoHolder);
                }
                for (const api::FormattedLink &screenshotLink : std::as_const(data.screenshots))
                {
                    auto screenshotHolder = new ImageHolder(QSize(271, 152), screenshotLink,apiClient, ui->mediaScrollAreaContents);
                    ui->mediaScrollAreaContentsLayout->addWidget(screenshotHolder);
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
                                emit navigate({Page::CATALOG_PRODUCT_PAGE, editionId});
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

            QStringList genres(data.tags.count());
            for (int i = 0; i < data.tags.count(); i++)
            {
                genres[data.tags[i].level - 1] = data.tags[i].name;
            }
            ui->genreInfoLabel->setText(genres.join(" - "));
            QStringList tags(data.properties.count());
            for (int i = 0; i < data.properties.count(); i++)
            {
                tags[i] = data.properties[i].name;
            }
            ui->tagsInfoLabel->setText(tags.join(", "));
            ui->supportedOSInfoLabel->setText(supportedOSInfo.join(", "));
            if (data.globalReleaseDate.isNull())
            {
                ui->releaseDateLabel->setVisible(false);
                ui->releaseDateInfoLabel->setVisible(false);
                ui->releaseDateInfoLabel->clear();
            }
            else
            {
                ui->releaseDateInfoLabel->setText(data.globalReleaseDate.date().toString("MMMM, d yyyy"));
                ui->releaseDateLabel->setVisible(true);
                ui->releaseDateInfoLabel->setVisible(true);
            }
            ui->companyInfoLabel->setText(QString("%1 / %2").arg(data.developers[0], data.publisher));
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
                ui->sizeInfoLabel->setText(data.size > 1000 ? QString::number(data.size / 1000.0, 'g', 1) : QString::number(data.size) + " MB");
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
            if (data.ratings.contains("PEGI"))
            {
                auto contentRating = data.ratings["PEGI"];
                ui->contentRatingLabel->setVisible(true);
                ui->contentRatingInfoLabel->setVisible(true);
                auto contentRatingDescription = QString("%1 Rating: %2%3").arg(
                            "PEGI",
                            QString::number(contentRating.ageRating) + "+",
                            contentRating.contentDescriptors.empty()
                                ? " (" + contentRating.contentDescriptors.join(", ") + ")"
                                : "");
                ui->contentRatingInfoLabel->setText(contentRatingDescription);
            }
            else
            {
                ui->contentRatingLabel->setVisible(false);
                ui->contentRatingInfoLabel->setVisible(false);
                ui->contentRatingInfoLabel->clear();
                ui->contentRatingIconLabel->clear();
            }
            for (const api::MetaTag &feature : std::as_const(data.features))
            {
                ui->gameFeaturesLayout->addWidget(new FeatureItem(feature, ui->gameDetails));
            }

            QLocale systemLocale = QLocale::system();
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
                                auto productItem = new SimpleProductItem(item.id, ui->seriesResultPage);
                                productItem->setCover(item.imageLink, apiClient);
                                productItem->setTitle(item.title);

                                connect(apiClient, &api::GogApiClient::authenticated,
                                        productItem, &SimpleProductItem::switchUiAuthenticatedState);
                                productItem->switchUiAuthenticatedState(apiClient->isAuthenticated());

                                connect(productItem, &SimpleProductItem::navigateToProduct, this, [this](unsigned long long productId)
                                {
                                    emit navigate({Page::CATALOG_PRODUCT_PAGE, productId});
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

            ui->descriptionView->setHtml("<style>body{max-width:100%;}img{max-width:100%;}</style>" + data.description);
            ui->featuresLabel->setText(data.featuresDescription);
            ui->featuresLabel->setVisible(!data.featuresDescription.isNull());
            ui->copyrightsLabel->setText(data.copyrights);
            ui->copyrightsLabel->setVisible(!data.copyrights.isNull());

            ui->contentStack->setCurrentWidget(ui->mainPage);
        }
        else if (networkReply->error() != QNetworkReply::OperationCanceledError)
        {
            qDebug() << networkReply->error()
                     << networkReply->errorString()
                     << QString(networkReply->readAll()).toUtf8();
        }
        networkReply->deleteLater();
    });

    auto systemLocale = QLocale::system();
    auto currencyCode = systemLocale.currencySymbol(QLocale::CurrencyIsoCode);
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
                auto recommendationItem = new SimpleProductItem(recommendation.productId, ui->purchasedTogetherResultPage);
                recommendationItem->setCover(recommendation.details.imageHorizontalUrl, apiClient);
                recommendationItem->setTitle(recommendation.details.title);
                if (recommendation.pricing.priceSet)
                {
                    recommendationItem->setPrice(recommendation.pricing.basePrice, recommendation.pricing.finalPrice,
                                                 recommendation.pricing.currency);
                }

                connect(apiClient, &api::GogApiClient::authenticated,
                        recommendationItem, &SimpleProductItem::switchUiAuthenticatedState);
                recommendationItem->switchUiAuthenticatedState(apiClient->isAuthenticated());

                connect(recommendationItem, &SimpleProductItem::navigateToProduct, this, [this](unsigned long long productId)
                {
                    emit navigate({Page::CATALOG_PRODUCT_PAGE, productId});
                });
                ui->purchasedTogetherResultPage->layout()->addWidget(recommendationItem);
            }
            ui->purchasedTogetherStackedWidget->setCurrentWidget(ui->purchasedTogetherResultPage);
        }
        else if (networkReply->error() != QNetworkReply::OperationCanceledError)
        {
            qDebug() << networkReply->error()
                     << networkReply->errorString()
                     << QString(networkReply->readAll()).toUtf8();
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
                auto recommendationItem = new SimpleProductItem(recommendation.productId, ui->similarProductsResultPage);
                recommendationItem->setCover(recommendation.details.imageHorizontalUrl, apiClient);
                recommendationItem->setTitle(recommendation.details.title);
                if (recommendation.pricing.priceSet)
                {
                    recommendationItem->setPrice(recommendation.pricing.basePrice, recommendation.pricing.finalPrice,
                                                 recommendation.pricing.currency);
                }

                connect(apiClient, &api::GogApiClient::authenticated,
                        recommendationItem, &SimpleProductItem::switchUiAuthenticatedState);
                recommendationItem->switchUiAuthenticatedState(apiClient->isAuthenticated());

                connect(recommendationItem, &SimpleProductItem::navigateToProduct, this, [this](unsigned long long productId)
                {
                    emit navigate({Page::CATALOG_PRODUCT_PAGE, productId});
                });
                ui->similarProductsResultPage->layout()->addWidget(recommendationItem);
            }
            ui->similarProductsStackedWidget->setCurrentWidget(ui->similarProductsResultPage);
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
    ui->cartButton->setEnabled(authenticated);
    ui->seriesBuyButton->setEnabled(authenticated);
    ui->wishlistButton->setEnabled(authenticated);
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

void CatalogProductPage::updateUserReviews()
{
    ui->userReviewsStackedWidget->setCurrentWidget(ui->userReviewsLoaderPage);
    while (!ui->userReviewsContentsLayout->isEmpty())
    {
        auto item = ui->userReviewsContentsLayout->itemAt(0);
        ui->userReviewsContentsLayout->removeItem(item);
        item->widget()->deleteLater();
        delete item;
    }

    if (lastReviewsReply != nullptr)
    {
        lastReviewsReply->abort();
    }
    lastReviewsReply = apiClient->getProductReviews(id, QStringList(), reviewsOrder, reviewsPageSize, reviewsPage);
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
            ui->filteredRatingLabel->setText(QString("Filters based rating: %1/5").arg(QString::number(data.filteredAvgRating, 'g', 2)));
            for (const api::Review &review : std::as_const(data.items))
            {
                auto reviewItem = new ReviewItem(review, review.id == data.mostHelpfulReviewId,
                                                 apiClient,
                                                 ui->userReviewsResultsPage);
                ui->userReviewsContentsLayout->addWidget(reviewItem);
            }
            ui->userReviewsStackedWidget->setCurrentWidget(ui->userReviewsResultsPage);
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
    updateUserReviews();
}
