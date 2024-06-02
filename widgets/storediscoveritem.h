#ifndef STOREDISCOVERITEM_H
#define STOREDISCOVERITEM_H

#include <QWidget>

#include "../api/gogapiclient.h"

namespace Ui {
class StoreDiscoverItem;
}

class StoreDiscoverItem : public QWidget
{
    Q_OBJECT

public:
    explicit StoreDiscoverItem(unsigned long long id, QWidget *parent = nullptr);
    ~StoreDiscoverItem();

    void setCover(const QString &coverUrl, api::GogApiClient *apiClient);
    void setPreorder(bool preorder);
    void setPrice(double basePrice, double finalPrice,
                  unsigned char discount, bool free, const QString &currency);
    void setTitle(const QString &title);

signals:
    void navigateToProduct(unsigned long long id);

protected:
    void mousePressEvent(QMouseEvent *event) override;

private:
    unsigned long long id;
    QNetworkReply *imageReply;
    Ui::StoreDiscoverItem *ui;
};

#endif // STOREDISCOVERITEM_H
