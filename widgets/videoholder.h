#ifndef VIDEOHOLDER_H
#define VIDEOHOLDER_H

#include <QSize>
#include <QWidget>

#include "../api/gogapiclient.h"

namespace Ui {
class VideoHolder;
}

class VideoHolder : public QWidget
{
    Q_OBJECT

public:
    explicit VideoHolder(QSize size,
                         const QString &thumbnailLink,
                         api::GogApiClient *apiClient,
                         QWidget *parent = nullptr);
    ~VideoHolder();

signals:
    void clicked();

protected:
    void mousePressEvent(QMouseEvent *event) override;

private:
    Ui::VideoHolder *ui;
};

#endif // VIDEOHOLDER_H
