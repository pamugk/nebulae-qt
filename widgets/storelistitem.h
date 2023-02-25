#ifndef STORELISTITEM_H
#define STORELISTITEM_H

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
    explicit StoreListItem(const CatalogProduct &data,
                           GogApiClient *apiClient,
                           QWidget *parent = nullptr);
    ~StoreListItem();

private:
    QNetworkReply *imageReply;
    Ui::StoreListItem *ui;
};

#endif // STORELISTITEM_H
