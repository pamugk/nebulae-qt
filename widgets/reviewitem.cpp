#include "reviewitem.h"
#include "ui_reviewitem.h"

ReviewItem::ReviewItem(const api::Review &data, bool isMostHelpful,
                       api::GogApiClient *apiClient,
                       QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ReviewItem)
{
    ui->setupUi(this);
    ui->mostHelpfulLabel->setVisible(isMostHelpful);
    ui->ratingLabel->setText(QString::number(data.rating, 'g', 2) + "/5");
    ui->titleLabel->setText(data.contentTitle);
    QStringList subheader;
    subheader.append(data.date.toString("MMMM, d yyyy"));
    foreach (const QString &label, data.labels)
    {
        if (label == "verified_owner")
        {
            subheader.append("Verified owner");
        }
    }
    ui->subheaderLabel->setText(subheader.join(" • "));
    ui->descriptionLabel->setText(data.contentDescription);
    ui->helpfulInfoLabel->setText(QString("(%1 of %2 users found this helpful))")
                                  .arg(QString::number(data.upvotes), QString::number(data.upvotes + data.downvotes)));

    ui->userLayout->setAlignment(Qt::AlignTop);
    avatarReply = apiClient->getAnything(data.reviewer.avatar["medium_2x"]);
    connect(avatarReply, &QNetworkReply::finished, this, [this]() {
        auto networkReply = avatarReply;
        avatarReply = nullptr;
        if (networkReply->error() == QNetworkReply::NoError)
        {
            QPixmap image;
            image.loadFromData(networkReply->readAll());
            ui->userAvatarLabel->setPixmap(image.scaled(ui->userAvatarLabel->size()));
        }
        networkReply->deleteLater();
    });
    ui->userNameLabel->setText(data.reviewer.username);
    ui->userGamesLabel->setText("Games: " + QString::number(data.reviewer.counters.games));
    ui->userReviewsLabel->setText("Reviews: " + QString::number(data.reviewer.counters.games));
}

ReviewItem::~ReviewItem()
{
    if (avatarReply != nullptr)
    {
        avatarReply->abort();
    }
    delete ui;
}
