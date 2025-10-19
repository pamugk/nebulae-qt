#include "reviewitem.h"
#include "ui_reviewitem.h"

#include <QNetworkReply>

ReviewItem::ReviewItem(const api::Review &data, bool isMostHelpful,
                       api::GogApiClient *apiClient,
                       QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ReviewItem)
{
    ui->setupUi(this);
    ui->mostHelpfulLabel->setVisible(isMostHelpful);
    if (isMostHelpful)
    {
        setStyleSheet("background: rgb(237, 237, 237);");
    }
    ui->ratingLabel->setText(QString::number(data.rating, 'g', 2) + "/5");
    ui->titleLabel->setText(data.contentTitle);
    QStringList subheader;
    subheader << data.date.toString("MMMM, d yyyy");
    for (const QString &label : std::as_const(data.labels))
    {
        if (label == "verified_owner")
        {
            subheader << "Verified owner";
        }
    }
    ui->subheaderLabel->setText(subheader.join(" • "));
    ui->descriptionLabel->setText(data.contentDescription);
    ui->helpfulInfoLabel->setText(QString("(%1 of %2 users found this helpful)")
                                  .arg(QString::number(data.upvotes), QString::number(data.upvotes + data.downvotes)));

    ui->userLayout->setAlignment(Qt::AlignTop);
    QNetworkReply *avatarReply = apiClient->getAnything(data.reviewer.avatar["medium_2x"]);
    connect(this, &QObject::destroyed, avatarReply, &QNetworkReply::abort);
    connect(avatarReply, &QNetworkReply::finished, this, [this, avatarReply]() {
        if (avatarReply->error() == QNetworkReply::NoError)
        {
            QPixmap image;
            image.loadFromData(avatarReply->readAll());
            ui->userAvatarLabel->setPixmap(image.scaled(ui->userAvatarLabel->size()));
        }
        else if (avatarReply->error() != QNetworkReply::OperationCanceledError)
        {
            qDebug() << avatarReply->error()
                     << avatarReply->errorString()
                     << QString(avatarReply->readAll()).toUtf8();
        }
    });
    connect(avatarReply, &QNetworkReply::finished, avatarReply, &QNetworkReply::deleteLater);

    ui->userNameLabel->setText(data.reviewer.username);
    ui->userGamesLabel->setText("Games: " + QString::number(data.reviewer.counters.games));
    ui->userReviewsLabel->setText("Reviews: " + QString::number(data.reviewer.counters.games));
}

ReviewItem::~ReviewItem()
{
    delete ui;
}
