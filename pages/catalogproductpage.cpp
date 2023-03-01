#include "catalogproductpage.h"
#include "ui_catalogproductpage.h"

#include <QCheckBox>
#include <QJsonDocument>
#include <QNetworkReply>

#include "../api/utils/catalogproductserialization.h"
#include "../layouts/flowlayout.h"
#include "../widgets/bonusitem.h"
#include "../widgets/checkeditem.h"
#include "../widgets/featureitem.h"

CatalogProductPage::CatalogProductPage(QWidget *parent) :
    BasePage(parent),
    ui(new Ui::CatalogProductPage)
{
    ui->setupUi(this);

    ui->contentRatingWarningPageLayout->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);

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

void CatalogProductPage::setProductId(quint64 id)
{
    this->id = id;
}

void CatalogProductPage::clear()
{
    ui->contentStack->setCurrentWidget(ui->loaderPage);
    ui->descriptionView->setUrl(QUrl("about:blank"));
    ui->editionsComboBox->clear();

    ui->windowsLabel->setVisible(false);
    ui->macOsLabel->setVisible(false);
    ui->linuxLabel->setVisible(false);

    while (!ui->gameFeaturesLayout->isEmpty())
    {
        auto item = ui->gameFeaturesLayout->itemAt(0);
        ui->gameFeaturesLayout->removeItem(item);
        item->widget()->deleteLater();
        delete item;
    }
    while (!ui->languagesLayout->isEmpty())
    {
        auto item = ui->languagesLayout->itemAt(0);
        ui->languagesLayout->removeItem(item);
        item->widget()->deleteLater();
        delete item;
    }

    while (!ui->productGoodiesSection->layout()->isEmpty())
    {
        auto item = ui->productGoodiesSection->layout()->itemAt(0);
        ui->productGoodiesSection->layout()->removeItem(item);
        item->widget()->deleteLater();
        delete item;
    }
    while (!ui->goodiesComparisonSectionGrid->isEmpty())
    {
        auto item = ui->goodiesComparisonSectionGrid->itemAt(0);
        ui->goodiesComparisonSectionGrid->removeItem(item);
        item->widget()->deleteLater();
        delete item;
    }

    ui->systemRequirementsTabWidget->setTabVisible(ui->systemRequirementsTabWidget->indexOf(ui->windowsTab), false);
    while (!ui->windowsTabLayout->isEmpty())
    {
        auto item = ui->windowsTabLayout->itemAt(0);
        ui->windowsTabLayout->removeItem(item);
        item->widget()->deleteLater();
        delete item;
    }
    ui->systemRequirementsTabWidget->setTabVisible(ui->systemRequirementsTabWidget->indexOf(ui->macOsTab), false);
    while (!ui->macOsTabLayout->isEmpty())
    {
        auto item = ui->macOsTabLayout->itemAt(0);
        ui->macOsTabLayout->removeItem(item);
        item->widget()->deleteLater();
        delete item;
    }
    ui->systemRequirementsTabWidget->setTabVisible(ui->systemRequirementsTabWidget->indexOf(ui->linuxTab), false);
    while (!ui->linuxTabLayout->isEmpty())
    {
        auto item = ui->linuxTabLayout->itemAt(0);
        ui->linuxTabLayout->removeItem(item);
        item->widget()->deleteLater();
        delete item;
    }
    id = 0;
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

void CatalogProductPage::initialize()
{
    auto networkReply = apiClient->getCatalogProductInfo(id, "en-US");
    connect(networkReply, &QNetworkReply::finished, this, [this, networkReply]()
    {
        if (networkReply->error() == QNetworkReply::NoError)
        {
            auto resultJson = QJsonDocument::fromJson(QString(networkReply->readAll()).toUtf8()).object();
            api::GetCatalogProductInfoResponse data;
            parseCatalogProductInfoResponse(resultJson, data);

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

                    api::Bonus bonus;
                    foreach (bonus, data.bonuses)
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
                        connect(editionCheckbox, &QCheckBox::setChecked, this, [this, editionId](bool checked)
                        {
                            if (checked)
                            {
                                emit navigateToDestination({Page::CATALOG_PRODUCT_PAGE, editionId});
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

            ui->ratingLabel->setText("0/5");

            ui->discountLabel->setVisible(false);
            ui->oldPriceLabel->setVisible(false);

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
            for (int i = 0; i < data.features.count(); i++)
            {
                ui->gameFeaturesLayout->addWidget(new FeatureItem(data.features[i], ui->gameDetails));
            }
            for (int i = 0; i < data.localizations.count(); i++)
            {
                auto languageLabel = new QLabel(data.localizations[i].language.name, ui->gameDetails);
                languageLabel->setStyleSheet("font-size: 11pt;");
                ui->languagesLayout->addWidget(languageLabel, i, 0);
                ui->languagesLayout->addWidget(new CheckedItem("audio", data.localizations[i].localizedAuio, ui->gameDetails), i, 1);
                ui->languagesLayout->addWidget(new CheckedItem("text", data.localizations[i].localizedText, ui->gameDetails), i, 2);
            }

            ui->descriptionView->setHtml("<style>body{max-width:100%;}img{max-width:100%;}</style>" + data.description);
            ui->featuresLabel->setText(data.featuresDescription);
            ui->featuresLabel->setVisible(!data.featuresDescription.isNull());
            ui->copyrightsLabel->setText(data.copyrights);
            ui->copyrightsLabel->setVisible(!data.copyrights.isNull());
            ui->contentStack->setCurrentWidget(ui->mainPage);

            networkReply->deleteLater();
        }
    });
    connect(networkReply, &QNetworkReply::errorOccurred, this, [this, networkReply](QNetworkReply::NetworkError error)
    {
        if (error != QNetworkReply::NoError)
        {
            networkReply->deleteLater();
        }
    });
}

void CatalogProductPage::on_acceptContentWarningButton_clicked()
{
    ui->contentStack->setCurrentWidget(ui->mainPage);
}


void CatalogProductPage::on_goBackButton_clicked()
{
    emit navigateToDestination({Page::ALL_GAMES, QVariant()});
}

void CatalogProductPage::descriptionViewContentsSizeChanged(const QSizeF &size)
{
    ui->descriptionView->setFixedHeight(size.height());
}

