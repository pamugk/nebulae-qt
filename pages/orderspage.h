#ifndef ORDERSPAGE_H
#define ORDERSPAGE_H

#include <QWidget>

#include "./basepage.h"

namespace Ui {
class OrdersPage;
}

class OrdersPage : public BasePage
{
    Q_OBJECT

public:
    explicit OrdersPage(QWidget *parent = nullptr);
    ~OrdersPage();

    virtual void setApiClient(GogApiClient *apiClient) override;

private:
    Ui::OrdersPage *ui;

public slots:
    virtual void clear() override;
    virtual void initialize() override;
};

#endif // ORDERSPAGE_H
