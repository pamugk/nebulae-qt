#ifndef VIDEOHOLDER_H
#define VIDEOHOLDER_H

#include <QNetworkReply>
#include <QSize>
#include <QWidget>

#include "../api/gogapiclient.h"
#include "../api/models/catalogproductinfo.h"

namespace Ui {
class VideoHolder;
}

class VideoHolder : public QWidget
{
    Q_OBJECT

public:
    explicit VideoHolder(QSize size,
                         const api::ThumbnailedVideo &data,
                         api::GogApiClient *apiClient,
                         QWidget *parent = nullptr);
    ~VideoHolder();

signals:
    void clicked();

protected:
    void mousePressEvent(QMouseEvent *event) override;

private:
    QNetworkReply *thumbnailReply;
    Ui::VideoHolder *ui;
};

#endif // VIDEOHOLDER_H
