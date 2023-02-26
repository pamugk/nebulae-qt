#ifndef ORDERGROUP_H
#define ORDERGROUP_H

#include <QWidget>

#include "../api/models/order.h"
#include "../api/gogapiclient.h"

namespace Ui {
class OrderGroup;
}

class OrderGroup : public QWidget
{
    Q_OBJECT

public:
    explicit OrderGroup(const api::Order &data,
                        api::GogApiClient *apiClient,
                        QWidget *parent = nullptr);
    ~OrderGroup();

private slots:
    void on_infoButton_clicked();

private:
    Ui::OrderGroup *ui;
};

#endif // ORDERGROUP_H
