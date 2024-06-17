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
    explicit StoreDiscoverItem( QWidget *parent = nullptr);
    ~StoreDiscoverItem();

    void setCover(const QString &coverUrl, api::GogApiClient *apiClient);
    void setOwned(bool owned);
    void setPreorder(bool preorder);
    void setPrice(double basePrice, double finalPrice,
                  unsigned char discount, bool free, const QString &currency);
    void setTitle(const QString &title);
    void setWishlisted(bool wishlisted);

signals:
    void clicked();

public slots:
    void switchUiAuthenticatedState(bool authenticated);

protected:
    void mousePressEvent(QMouseEvent *event) override;

private:
    QNetworkReply *imageReply;
    Ui::StoreDiscoverItem *ui;
};

#endif // STOREDISCOVERITEM_H
