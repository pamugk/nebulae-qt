#ifndef ORDERITEM_H
#define ORDERITEM_H

#include <QNetworkReply>
#include <QWidget>

#include "../api/models/order.h"
#include "../api/gogapiclient.h"

namespace Ui {
class OrderItem;
}

class OrderItem : public QWidget
{
    Q_OBJECT

public:
    explicit OrderItem(const OrderProduct &data,
                       GogApiClient *apiClient,
                       QWidget *parent = nullptr);
    ~OrderItem();

private:
    QNetworkReply *imageReply;
    Ui::OrderItem *ui;
};

#endif // ORDERITEM_H
