#ifndef WISHLISTITEM_H
#define WISHLISTITEM_H

#include <QWidget>

namespace Ui {
class WishlistItem;
}

class WishlistItem : public QWidget
{
    Q_OBJECT

public:
    explicit WishlistItem(QWidget *parent = nullptr);
    ~WishlistItem();

private:
    Ui::WishlistItem *ui;
};

#endif // WISHLISTITEM_H
