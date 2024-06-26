#ifndef IMAGEHOLDER_H
#define IMAGEHOLDER_H

#include <QMouseEvent>
#include <QSize>
#include <QWidget>

#include "../api/gogapiclient.h"

namespace Ui {
class ImageHolder;
}

class ImageHolder : public QWidget
{
    Q_OBJECT

public:
    explicit ImageHolder(QSize size,
                         const QString &url,
                         api::GogApiClient *apiClient,
                         QWidget *parent = nullptr);
    ~ImageHolder();

signals:
    void clicked();

protected:
    void mousePressEvent(QMouseEvent *event) override;

private:
    Ui::ImageHolder *ui;
};

#endif // IMAGEHOLDER_H
