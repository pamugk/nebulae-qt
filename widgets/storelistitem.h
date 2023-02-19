#ifndef STORELISTITEM_H
#define STORELISTITEM_H

#include <QWidget>

namespace Ui {
class StoreListItem;
}

class StoreListItem : public QWidget
{
    Q_OBJECT

public:
    explicit StoreListItem(QWidget *parent = nullptr);
    ~StoreListItem();

private:
    Ui::StoreListItem *ui;
};

#endif // STORELISTITEM_H
