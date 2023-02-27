#ifndef WISHLISTITEM_H
#define WISHLISTITEM_H

#include <QMouseEvent>
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
    explicit WishlistItem(const api::Product &data,
                          api::GogApiClient *apiClient,
                          QWidget *parent = nullptr);
    ~WishlistItem();

signals:
    void navigateToProduct(quint64 id);

protected:
    void mousePressEvent(QMouseEvent *event) override;

private:
    quint64 itemId;
    QNetworkReply *imageReply;
    Ui::WishlistItem *ui;
};

#endif // WISHLISTITEM_H
