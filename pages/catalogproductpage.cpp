#include "catalogproductpage.h"
#include "ui_catalogproductpage.h"

#include <QJsonDocument>
#include <QNetworkReply>

#include "../api/utils/catalogproductserialization.h"
#include "../widgets/checkeditem.h"
#include "../widgets/featureitem.h"

CatalogProductPage::CatalogProductPage(QWidget *parent) :
    BasePage(parent),
    ui(new Ui::CatalogProductPage)
{
    ui->setupUi(this);

    ui->contentRatingWarningPageLayout->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    ui->productPriceLayout->setAlignment(Qt::AlignTop);
    ui->gameDetailsLayout->setAlignment(Qt::AlignTop);
    ui->gameFeaturesLayout->setAlignment(Qt::AlignTop);
    ui->languagesLayout->setAlignment(Qt::AlignTop);
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
    ui->descriptionView->setUrl(QUrl("about:blank"));
    ui->editionsComboBox->clear();
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
    id = 0;
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

            QStringList supportedOSInfo(data.supportedOperatingSystems.count());
            for (int i = 0; i < data.supportedOperatingSystems.count(); i++)
            {
                supportedOSInfo[i] = data.supportedOperatingSystems[i].versions;
            }

            ui->titleLabel->setText(data.title);

            ui->ratingLabel->setText("0/5");

            ui->discountLabel->setVisible(false);
            ui->oldPriceLabel->setVisible(false);
            ui->editionsComboBox->setVisible(!data.editions.isEmpty());
            for (int i = 0; i < data.editions.count(); i++)
            {
                ui->editionsComboBox->addItem(data.editions[i].name);
            }
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

