#ifndef WISHLISTITEM_H
#define WISHLISTITEM_H

#include <QWidget>

#include "../api/models/product.h"

namespace Ui {
class WishlistItem;
}

class WishlistItem : public QWidget
{
    Q_OBJECT

public:
    explicit WishlistItem(const Product &data, QWidget *parent = nullptr);
    ~WishlistItem();

private:
    Ui::WishlistItem *ui;
};

#endif // WISHLISTITEM_H
