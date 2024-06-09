#ifndef DEPENDENTPRODUCTITEM_H
#define DEPENDENTPRODUCTITEM_H

#include <QMouseEvent>
#include <QNetworkReply>
#include <QWidget>

#include "../api/gogapiclient.h"
#include "../api/models/catalogproductinfo.h"

namespace Ui {
class DependentProductItem;
}

class DependentProductItem : public QWidget
{
    Q_OBJECT

public:
    explicit DependentProductItem(const api::GetCatalogProductInfoResponse &data,
                                  api::GogApiClient *apiClient,
                                  QWidget *parent = nullptr);
    ~DependentProductItem();

signals:
    void clicked();

public slots:
    void switchUiAuthenticatedState(bool authenticated);

protected:
    void mousePressEvent(QMouseEvent *event) override;

private:
    QNetworkReply *imageReply;
    Ui::DependentProductItem *ui;
};

#endif // DEPENDENTPRODUCTITEM_H
