#include "catalogproductmediadialog.h"
#include "ui_catalogproductmediadialog.h"

#include <QNetworkReply>

CatalogProductMediaDialog::CatalogProductMediaDialog(const QVector<api::ThumbnailedVideo> &videos,
                                                     const QVector<api::FormattedLink> &screenshots,
                                                     api::GogApiClient *apiClient,
                                                     QWidget *parent) :
    QDialog(parent),
    apiClient(apiClient),
    screenshots(screenshots),
    videos(videos),
    ui(new Ui::CatalogProductMediaDialog)
{
    ui->setupUi(this);
}

CatalogProductMediaDialog::~CatalogProductMediaDialog()
{
    delete ui;
}

void CatalogProductMediaDialog::viewMedia(std::size_t index)
{
    currentItem = index;
    ui->showPreviousButton->setEnabled(index > 0);
    ui->showNextButton->setEnabled(index < (screenshots.count() + videos.count() - 1));

    emit updatingImage();
    if (index < videos.count())
    {
        const api::ThumbnailedVideo &currentVideo = videos[index];
        ui->videoView->setUrl(QUrl(currentVideo.videoLink));
        ui->contentStackedWidget->setCurrentWidget(ui->videoPage);
    }
    else
    {
        ui->videoView->setUrl(QUrl("about:blank"));
        const api::FormattedLink &currentImage = screenshots[index - videos.count()];
        QNetworkReply *imageReply = apiClient->getAnything(currentImage.templated
                                            ? QString(currentImage.href).replace("{formatter}", currentImage.formatters[2])
                                            : currentImage.href);
        ui->contentStackedWidget->setCurrentWidget(ui->loadingPage);
        connect(this, &QObject::destroyed, imageReply, &QNetworkReply::abort);
        connect(this, &CatalogProductMediaDialog::updatingImage, imageReply, &QNetworkReply::abort);
        connect(imageReply, &QNetworkReply::finished, this, [this, imageReply]()
        {
            if (imageReply->error() == QNetworkReply::NoError)
            {
                QPixmap image;
                image.loadFromData(imageReply->readAll());
                ui->imageLabel->setPixmap(image);
                ui->contentStackedWidget->setCurrentWidget(ui->imagePage);
            }
            else if (imageReply->error() != QNetworkReply::OperationCanceledError)
            {
                qDebug() << imageReply->error()
                         << imageReply->errorString()
                         << QString(imageReply->readAll()).toUtf8();
            }
        });
        connect(imageReply, &QNetworkReply::finished, imageReply, &QNetworkReply::deleteLater);
    }
}

void CatalogProductMediaDialog::on_showPreviousButton_clicked()
{
    viewMedia(currentItem - 1);
}


void CatalogProductMediaDialog::on_showNextButton_clicked()
{
    viewMedia(currentItem + 1);
}

