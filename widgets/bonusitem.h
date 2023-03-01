#ifndef BONUSITEM_H
#define BONUSITEM_H

#include <QWidget>

#include "../api/models/catalogproductinfo.h"

namespace Ui {
class BonusItem;
}

class BonusItem : public QWidget
{
    Q_OBJECT

public:
    explicit BonusItem(const api::Bonus &data, QWidget *parent = nullptr);
    ~BonusItem();

private:
    Ui::BonusItem *ui;
};

#endif // BONUSITEM_H
