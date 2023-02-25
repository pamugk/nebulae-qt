#ifndef WISHLISTITEM_H
#define WISHLISTITEM_H

#include <QNetworkReply>
#include <QWidget>

#include "../api/models/product.h"
#include "../api/gogapiclient.h"

namespace Ui {
class WishlistItem;
}

class WishlistItem : public QWidget
{
    Q_OBJECT

public:
    explicit WishlistItem(const Product &data,
                          GogApiClient *apiClient,
                          QWidget *parent = nullptr);
    ~WishlistItem();

private:
    QNetworkReply *imageReply;
    Ui::WishlistItem *ui;
};

#endif // WISHLISTITEM_H
