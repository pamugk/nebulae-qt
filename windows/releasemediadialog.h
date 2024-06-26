#ifndef RELEASEMEDIADIALOG_H
#define RELEASEMEDIADIALOG_H

#include <QDialog>

#include "../api/gogapiclient.h"
#include "../api/models/game.h"

namespace Ui {
class ReleaseMediaDialog;
}

class ReleaseMediaDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ReleaseMediaDialog(const QVector<api::NamedVideo> &videos,
                                const QVector<QString> &screenshots,
                                api::GogApiClient *apiClient,
                                QWidget *parent = nullptr);
    ~ReleaseMediaDialog();

public slots:
    void viewMedia(std::size_t index);

private slots:
    void on_showPreviousButton_clicked();
    void on_showNextButton_clicked();

private:
    api::GogApiClient *apiClient;
    std::size_t currentItem;
    QNetworkReply *imageReply;
    const QVector<QString> &screenshots;
    const QVector<api::NamedVideo> &videos;
    Ui::ReleaseMediaDialog *ui;
};

#endif // RELEASEMEDIADIALOG_H
