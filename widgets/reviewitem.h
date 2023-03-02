#ifndef REVIEWITEM_H
#define REVIEWITEM_H

#include <QNetworkReply>
#include <QWidget>

#include "../api/gogapiclient.h"
#include "../api/models/review.h"

namespace Ui {
class ReviewItem;
}

class ReviewItem : public QWidget
{
    Q_OBJECT

public:
    explicit ReviewItem(const api::Review &data, bool isMostHelpful,
                        api::GogApiClient *apiClient,
                        QWidget *parent = nullptr);
    ~ReviewItem();

private:
    QNetworkReply *avatarReply;
    Ui::ReviewItem *ui;
};

#endif // REVIEWITEM_H
