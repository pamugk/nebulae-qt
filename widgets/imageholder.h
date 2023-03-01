#ifndef IMAGEHOLDER_H
#define IMAGEHOLDER_H

#include <QNetworkReply>
#include <QSize>
#include <QWidget>

#include "../api/gogapiclient.h"
#include "../api/models/catalogproductinfo.h"

namespace Ui {
class ImageHolder;
}

class ImageHolder : public QWidget
{
    Q_OBJECT

public:
    explicit ImageHolder(QSize size,
                         const api::FormattedLink &data,
                         api::GogApiClient *apiClient,
                         QWidget *parent = nullptr);
    ~ImageHolder();

private:
    QNetworkReply *imageReply;
    Ui::ImageHolder *ui;
};

#endif // IMAGEHOLDER_H
