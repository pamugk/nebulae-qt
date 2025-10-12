#ifndef STORESALECARD_H
#define STORESALECARD_H

#include <QWidget>

#include "../api/gogapiclient.h"
#include "../api/models/store.h"

namespace Ui {
class StoreSaleCard;
}

class StoreSaleCard : public QWidget
{
    Q_OBJECT

public:
    explicit StoreSaleCard(const api::StoreNowOnSaleTabCard &data,
                           api::GogApiClient *apiClient,
                           QWidget *parent = nullptr);
    ~StoreSaleCard();

signals:
    void navigateToItem();

protected:
    void mousePressEvent(QMouseEvent *event) override;

private:
    QNetworkReply *imageReply;
    Ui::StoreSaleCard *ui;
};

#endif // STORESALECARD_H
