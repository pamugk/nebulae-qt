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
#include "../api/utils/reviewserialization.h"

#include "../layouts/flowlayout.h"
#include "../widgets/bonusitem.h"
#include "../widgets/checkeditem.h"
#include "../widgets/featureitem.h"
#include "../widgets/imageholder.h"
#include "../widgets/pagination.h"
#include "../widgets/reviewitem.h"
#include "../widgets/videoholder.h"

CatalogProductPage::CatalogProductPage(QWidget *parent) :
    BasePage(parent),
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
    auto averageRatingResponse = apiClient->getProductAverageRating(id);
    connect(averageRatingResponse, &QNetworkReply::finished, this, [this, averageRatingResponse]()
    {
        if (averageRatingResponse->error() == QNetworkReply::NoError)
        {
            auto resultJson = QJsonDocument::fromJson(QString(averageRatingResponse->readAll()).toUtf8()).object();
            api::GetRatingResponse data;
            parseRatingResponse(resultJson, data);

            if (data.count > 0)
            {
                ui->starLabel->setVisible(true);
                ui->ratingLabel->setText(QString::number(data.value, 'g', 2) + "/5");
                ui->ratingLabel->setVisible(true);
            }

            averageRatingResponse->deleteLater();
        }
    });
    connect(averageRatingResponse, &QNetworkReply::errorOccurred, this, [this, averageRatingResponse](QNetworkReply::NetworkError error)
    {
        if (error != QNetworkReply::NoError)
        {
            averageRatingResponse->deleteLater();
        }
    });

    auto averageOwnerRatingResponse = apiClient->getProductAverageRating(id, "verified_owner");
    connect(averageOwnerRatingResponse, &QNetworkReply::finished, this, [this, averageOwnerRatingResponse]()
    {
        if (averageOwnerRatingResponse->error() == QNetworkReply::NoError)
        {
            auto resultJson = QJsonDocument::fromJson(QString(averageOwnerRatingResponse->readAll()).toUtf8()).object();
            api::GetRatingResponse data;
            parseRatingResponse(resultJson, data);

            ui->ownersRatingLabel->setText(QString("Verified owners rating: %1/5").arg(QString::number(data.value, 'g', 2)));
            ui->ownersRatingLabel->setVisible(true);

            averageOwnerRatingResponse->deleteLater();
        }
    });
    connect(averageOwnerRatingResponse, &QNetworkReply::errorOccurred, this, [this, averageOwnerRatingResponse](QNetworkReply::NetworkError error)
    {
        if (error != QNetworkReply::NoError)
        {
            averageOwnerRatingResponse->deleteLater();
        }
    });

    auto countryCode = QLocale::territoryToCode(QLocale::system().territory());
    auto pricesNetworkReply = apiClient->getProductPrices(id, countryCode);
    connect(pricesNetworkReply, &QNetworkReply::finished, this, [this, pricesNetworkReply]()
    {
        if (pricesNetworkReply->error() == QNetworkReply::NoError)
        {
            auto resultJson = QJsonDocument::fromJson(QString(pricesNetworkReply->readAll()).toUtf8()).object();
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
            pricesNetworkReply->deleteLater();
        }
    });
    connect(pricesNetworkReply, &QNetworkReply::errorOccurred, this, [this, pricesNetworkReply](QNetworkReply::NetworkError error)
    {
        if (error != QNetworkReply::NoError)
        {
            pricesNetworkReply->deleteLater();
        }
    });

    updateUserReviews();

    auto mainNetworkReply = apiClient->getCatalogProductInfo(id, "en-US");
    connect(mainNetworkReply, &QNetworkReply::finished, this, [this, mainNetworkReply]()
    {
        if (mainNetworkReply->error() == QNetworkReply::NoError)
        {
            auto resultJson = QJsonDocument::fromJson(QString(mainNetworkReply->readAll()).toUtf8()).object();
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

            ui->descriptionView->setHtml("<style>body{max-width:100%;}img{max-width:100%;}</style>" + data.description);
            ui->featuresLabel->setText(data.featuresDescription);
            ui->featuresLabel->setVisible(!data.featuresDescription.isNull());
            ui->copyrightsLabel->setText(data.copyrights);
            ui->copyrightsLabel->setVisible(!data.copyrights.isNull());

            ui->contentStack->setCurrentWidget(ui->mainPage);

            mainNetworkReply->deleteLater();
        }
    });
    connect(mainNetworkReply, &QNetworkReply::errorOccurred, this, [this, mainNetworkReply](QNetworkReply::NetworkError error)
    {
        if (error != QNetworkReply::NoError)
        {
            mainNetworkReply->deleteLater();
        }
    });
}

void CatalogProductPage::switchUiAuthenticatedState(bool authenticated)
{
    ui->cartButton->setVisible(authenticated);
    ui->wishlistButton->setVisible(authenticated);
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
    auto reviewsReply = apiClient->getProductReviews(id, QStringList(), reviewsOrder, reviewsPageSize, reviewsPage);
    connect(reviewsReply, &QNetworkReply::finished, this, [this, reviewsReply]()
    {
        if (reviewsReply->error() == QNetworkReply::NoError)
        {
            auto resultJson = QJsonDocument::fromJson(QString(reviewsReply->readAll()).toUtf8()).object();
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
            reviewsReply->deleteLater();
        }
    });
    connect(reviewsReply, &QNetworkReply::errorOccurred, this, [this, reviewsReply](QNetworkReply::NetworkError error)
    {
        if (error != QNetworkReply::NoError)
        {
            qDebug() << error;
            reviewsReply->deleteLater();
        }
    });
}

void CatalogProductPage::changeUserReviewsPage(quint16 page)
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
