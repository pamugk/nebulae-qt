#include "catalogproductmediadialog.h"
#include "ui_catalogproductmediadialog.h"

#include <QNetworkReply>

CatalogProductMediaDialog::CatalogProductMediaDialog(const QVector<api::ThumbnailedVideo> &videos,
                                                     const QVector<api::FormattedLink> &screenshots,
                                                     api::GogApiClient *apiClient,
                                                     QWidget *parent) :
    QDialog(parent),
    apiClient(apiClient),
    imageReply(nullptr),
    screenshots(screenshots),
    videos(videos),
    ui(new Ui::CatalogProductMediaDialog)
{
    ui->setupUi(this);
}

CatalogProductMediaDialog::~CatalogProductMediaDialog()
{
    if (imageReply != nullptr)
    {
        imageReply->abort();
    }
    delete ui;
}

void CatalogProductMediaDialog::viewMedia(std::size_t index)
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
        const api::ThumbnailedVideo &currentVideo = videos[index];
        ui->videoView->setUrl(QUrl(currentVideo.videoLink));
        ui->contentStackedWidget->setCurrentWidget(ui->videoPage);
    }
    else
    {
        ui->videoView->setUrl(QUrl("about:blank"));
        const api::FormattedLink &currentImage = screenshots[index - videos.count()];
        imageReply = apiClient->getAnything(currentImage.templated
                                            ? QString(currentImage.href).replace("{formatter}", currentImage.formatters[2])
                                            : currentImage.href);
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

void CatalogProductMediaDialog::on_showPreviousButton_clicked()
{
    viewMedia(currentItem - 1);
}


void CatalogProductMediaDialog::on_showNextButton_clicked()
{
    viewMedia(currentItem + 1);
}

