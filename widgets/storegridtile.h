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
    explicit StoreGridTile(const api::CatalogProduct &data,
                           api::GogApiClient *apiClient,
                           QWidget *parent = nullptr);
    ~StoreGridTile();

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
    Ui::StoreGridTile *ui;
};

#endif // STOREGRIDTILE_H
