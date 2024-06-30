#include "releasemediadialog.h"
#include "ui_releasemediadialog.h"

#include <QNetworkReply>

ReleaseMediaDialog::ReleaseMediaDialog(const QVector<api::NamedVideo> &videos,
                                       const QVector<QString> &screenshots,
                                       api::GogApiClient *apiClient,
                                       QWidget *parent) :
    QDialog(parent),
    apiClient(apiClient),
    imageReply(nullptr),
    screenshots(screenshots),
    videos(videos),
    ui(new Ui::ReleaseMediaDialog)
{
    ui->setupUi(this);
}

ReleaseMediaDialog::~ReleaseMediaDialog()
{
    if (imageReply != nullptr)
    {
        imageReply->abort();
    }
    delete ui;
}

void ReleaseMediaDialog::viewMedia(std::size_t index)
{
    currentItem = index;
    ui->showPreviousButton->setEnabled(index > 0);
    ui->showNextButton->setEnabled(index < (screenshots.count() + videos.count() - 1));

    if (imageReply != nullptr)
    {
        imageReply->abort();
    }
    if (index < videos.count())
    {
        const api::NamedVideo &currentVideo = videos[index];
        QString videoLink;
        if (currentVideo.provider == "youtube")
        {
            videoLink = QString("https://www.youtube.com/embed/%1?wmode=opaque&rel=0").arg(currentVideo.videoId);
        }
        setWindowTitle(currentVideo.name.isNull() ? "Gallery" : currentVideo.name);
        ui->videoView->setUrl(QUrl(videoLink));
        ui->contentStackedWidget->setCurrentWidget(ui->videoPage);
    }
    else
    {
        ui->videoView->setUrl(QUrl("about:blank"));
        setWindowTitle("Gallery");
        const QString &currentImage = screenshots[index - videos.count()];
        QString imageUrl = QString(currentImage).replace("{formatter}", "_1600").replace("{ext}", "webp");
        imageReply = apiClient->getAnything(imageUrl);
        ui->contentStackedWidget->setCurrentWidget(ui->loadingPage);
        connect(imageReply, &QNetworkReply::finished, this, [this]()
        {
            auto networkReply = imageReply;
            imageReply = nullptr;
            if (networkReply->error() == QNetworkReply::NoError)
            {
                QPixmap image;
                image.loadFromData(networkReply->readAll());
                ui->imageLabel->setPixmap(image);
                ui->contentStackedWidget->setCurrentWidget(ui->imagePage);
            }
            networkReply->deleteLater();
        });
    }
}

void ReleaseMediaDialog::on_showPreviousButton_clicked()
{
    viewMedia(currentItem - 1);
}


void ReleaseMediaDialog::on_showNextButton_clicked()
{
    viewMedia(currentItem + 1);
}
