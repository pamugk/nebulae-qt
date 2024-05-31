#ifndef RECOMMENDATIONITEM_H
#define RECOMMENDATIONITEM_H

#include <QNetworkReply>
#include <QWidget>

#include "../api/gogapiclient.h"
#include "../api/models/recommendation.h"

namespace Ui {
class RecommendationItem;
}

class RecommendationItem : public QWidget
{
    Q_OBJECT

public:
    explicit RecommendationItem(const api::Recommendation &data,
                                api::GogApiClient *apiClient,
                                QWidget *parent = nullptr);
    ~RecommendationItem();

signals:
    void navigateToProduct(unsigned long long id);

protected:
    void mousePressEvent(QMouseEvent *event) override;

private:
    unsigned long long itemId;
    QNetworkReply *imageReply;
    Ui::RecommendationItem *ui;
};

#endif // RECOMMENDATIONITEM_H
