#ifndef CATALOGPRODUCTMEDIADIALOG_H
#define CATALOGPRODUCTMEDIADIALOG_H

#include <QDialog>
#include <QVector>

#include "../api/gogapiclient.h"
#include "../api/models/catalogproductinfo.h"

namespace Ui {
class CatalogProductMediaDialog;
}

class CatalogProductMediaDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CatalogProductMediaDialog(const QVector<api::ThumbnailedVideo> &videos,
                                       const QVector<api::FormattedLink> &screenshots,
                                       api::GogApiClient *apiClient,
                                       QWidget *parent = nullptr);
    ~CatalogProductMediaDialog();

public slots:
    void viewMedia(std::size_t index);

private slots:
    void on_showPreviousButton_clicked();

    void on_showNextButton_clicked();

private:
    api::GogApiClient *apiClient;
    std::size_t currentItem;
    QNetworkReply *imageReply;
    const QVector<api::FormattedLink> &screenshots;
    const QVector<api::ThumbnailedVideo> &videos;
    Ui::CatalogProductMediaDialog *ui;
};

#endif // CATALOGPRODUCTMEDIADIALOG_H
