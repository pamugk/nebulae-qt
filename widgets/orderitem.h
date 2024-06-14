#ifndef ORDERITEM_H
#define ORDERITEM_H

#include <QNetworkReply>
#include <QWidget>

#include "../api/gogapiclient.h"
#include "../api/models/order.h"

namespace Ui {
class OrderItem;
}

class OrderItem : public QWidget
{
    Q_OBJECT

public:
    explicit OrderItem(const api::OrderProduct &data,
                       api::GogApiClient *apiClient,
                       QWidget *parent = nullptr);
    ~OrderItem();

signals:
    void clicked();

protected:
    void mousePressEvent(QMouseEvent *event) override;

private:
    QNetworkReply *imageReply;
    Ui::OrderItem *ui;
};

#endif // ORDERITEM_H
