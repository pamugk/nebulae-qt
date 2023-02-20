#ifndef ORDERGROUP_H
#define ORDERGROUP_H

#include <QWidget>

namespace Ui {
class OrderGroup;
}

class OrderGroup : public QWidget
{
    Q_OBJECT

public:
    explicit OrderGroup(QWidget *parent = nullptr);
    ~OrderGroup();

private:
    Ui::OrderGroup *ui;
};

#endif // ORDERGROUP_H
