#ifndef STOREGRIDTILE_H
#define STOREGRIDTILE_H

#include <QNetworkReply>
#include <QWidget>

#include "../api/gogapiclient.h"
#include "../api/models/catalog.h"

namespace Ui {
class StoreGridTile;
}

class StoreGridTile : public QWidget
{
    Q_OBJECT

public:
    explicit StoreGridTile(const CatalogProduct &data,
                           GogApiClient *apiClient,
                           QWidget *parent = nullptr);
    ~StoreGridTile();

private:
    QNetworkReply *imageReply;
    Ui::StoreGridTile *ui;
};

#endif // STOREGRIDTILE_H
