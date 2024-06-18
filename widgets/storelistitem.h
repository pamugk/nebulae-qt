#ifndef STORELISTITEM_H
#define STORELISTITEM_H

#include <QMouseEvent>
#include <QNetworkReply>
#include <QWidget>

#include "../api/gogapiclient.h"
#include "../api/models/catalog.h"

namespace Ui {
class StoreListItem;
}

class StoreListItem : public QWidget
{
    Q_OBJECT

public:
    explicit StoreListItem(const api::CatalogProduct &data,
                           api::GogApiClient *apiClient,
                           QWidget *parent = nullptr);
    ~StoreListItem();

signals:
    void clicked();

public slots:
    void setOwned(bool owned);
    void setWishlisted(bool wishlisted);
    void switchUiAuthenticatedState(bool authenticated);

protected:
    void mousePressEvent(QMouseEvent *event) override;

private:
    QNetworkReply *imageReply;
    Ui::StoreListItem *ui;
};

#endif // STORELISTITEM_H
