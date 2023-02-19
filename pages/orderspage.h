#ifndef ORDERSPAGE_H
#define ORDERSPAGE_H

#include <QWidget>

namespace Ui {
class OrdersPage;
}

class OrdersPage : public QWidget
{
    Q_OBJECT

public:
    explicit OrdersPage(QWidget *parent = nullptr);
    ~OrdersPage();

private:
    Ui::OrdersPage *ui;
};

#endif // ORDERSPAGE_H
