#include "ownedproductgriditem.h"
#include "ui_ownedproductgriditem.h"

void setElidedText(QLabel *label, const QString &text)
{
    QFontMetrics metrics(label->font());
    QString elidedText = metrics.elidedText(text, Qt::ElideRight, label->width());
    label->setText(elidedText);
}

QString describeGameplayStats(const db::UserReleaseShortDetails &data)
{
    auto systemLocale = QLocale::system();
    QString gameplayStats;
    if (data.totalAchievementCount > 0)
    {
        gameplayStats += systemLocale.toString(data.unlockedAchievementCount / data.totalAchievementCount);
        gameplayStats += systemLocale.percent();
    }
    if (data.totalPlaytime > 0)
    {
        if (!gameplayStats.isEmpty())
        {
            gameplayStats += ' ';
        }
        gameplayStats += "🕒 ";
        if (data.totalPlaytime > 60)
        {
            gameplayStats += systemLocale.toString(data.totalPlaytime / 60);
            gameplayStats += " h. ";
        }
        gameplayStats += systemLocale.toString(data.totalPlaytime % 60);
        gameplayStats += " min.";
    }
    return gameplayStats;
}

OwnedProductGridItem::OwnedProductGridItem(const db::UserReleaseShortDetails &data,
                                           api::GogApiClient *apiClient,
                                           QWidget *parent) :
    QWidget(parent),
    additionalData({
                   std::make_pair(AdditionalInfo::COMPANY, data.developers),
                   std::make_pair(AdditionalInfo::GENRES, data.genres),
                   std::make_pair(AdditionalInfo::STATS, describeGameplayStats(data)),
                   std::make_pair(AdditionalInfo::TAGS, data.tags),
                   std::make_pair(AdditionalInfo::PLATFORM, data.platformId == "gog" ? "GOG.com" : "Other"),
                   }),
    title(data.title),
    ui(new Ui::OwnedProductGridItem)
{
    ui->setupUi(this);

    setElidedText(ui->titleLabel, title);

    ui->coverLabel->setToolTip(title);
    ui->titleLabel->setVisible(false);
    ui->titleLabel->setToolTip(title);
    ui->statusActionButton->setVisible(false);
    ui->ratingLabel->setVisible(false);
    ui->ratingLabel->setText(data.rating.has_value() ? (QString(data.rating.value(), u'★') + QString(5 - data.rating.value(), u'☆')) : "No rating");
    ui->additionalInfoLabel->setVisible(false);

    imageReply = apiClient->getAnything(QString(data.verticalCover).replace("{formatter}", "_glx_vertical_cover").replace("{ext}", "webp"));
    connect(imageReply, &QNetworkReply::finished, this, [this]()
    {
        auto networkReply = imageReply;
        imageReply = nullptr;
        if (networkReply->error() == QNetworkReply::NoError)
        {
            QPixmap image;
            image.loadFromData(networkReply->readAll());
            ui->coverLabel->setPixmap(image);
        }
        networkReply->deleteLater();
    });
}

OwnedProductGridItem::~OwnedProductGridItem()
{
    if (imageReply != nullptr)
    {
        imageReply->abort();
    }
    delete ui;
}

void OwnedProductGridItem::setAdditionalDataVisibility(bool visible)
{
    ui->additionalInfoLabel->setVisible(visible);
}

void OwnedProductGridItem::setAdditionalDataDisplayed(AdditionalInfo kind)
{
    QString displayedAdditionalData = additionalData[kind];
    setElidedText(ui->additionalInfoLabel, displayedAdditionalData);
    ui->additionalInfoLabel->setToolTip(displayedAdditionalData);
    this->displayedAdditionalData = kind;
}

void OwnedProductGridItem::setImageSize(const QSize &imageSize)
{
    this->setFixedWidth(imageSize.width());
    ui->coverLabel->setFixedSize(imageSize);
    setElidedText(ui->titleLabel, title);
    setElidedText(ui->additionalInfoLabel, additionalData[displayedAdditionalData]);
}

void OwnedProductGridItem::setRatingVisibility(bool visible)
{
    ui->ratingLabel->setVisible(visible);
}

void OwnedProductGridItem::setStatusVisibility(bool visible)
{
    ui->statusActionButton->setVisible(visible);
    setElidedText(ui->titleLabel, title);
}

void OwnedProductGridItem::setTitleVisibility(bool visible)
{
    ui->titleLabel->setVisible(visible);
}

void OwnedProductGridItem::mousePressEvent(QMouseEvent *event)
{
    emit clicked();
}
