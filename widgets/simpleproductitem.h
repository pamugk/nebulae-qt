#ifndef SIMPLEPRODUCTITEM_H
#define SIMPLEPRODUCTITEM_H

#include <QNetworkReply>
#include <QWidget>

#include "../api/gogapiclient.h"

namespace Ui {
class SimpleProductItem;
}

class SimpleProductItem : public QWidget
{
    Q_OBJECT

public:
    explicit SimpleProductItem(unsigned long long productId,
                               QWidget *parent = nullptr);
    ~SimpleProductItem();

    void setCover(const QString &coverUrl, api::GogApiClient *apiClient);
    void setTitle(const QString &title);
    void setPrice(unsigned int basePrice, unsigned int finalPrice,
                  const QString &currency);

signals:
    void navigateToProduct(unsigned long long id);

public slots:
    void switchUiAuthenticatedState(bool authenticated);

protected:
    void mousePressEvent(QMouseEvent *event) override;

private:
    unsigned long long itemId;
    QNetworkReply *imageReply;
    Ui::SimpleProductItem *ui;
};

#endif // SIMPLEPRODUCTITEM_H
