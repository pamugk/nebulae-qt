#ifndef ORDERSPAGE_H
#define ORDERSPAGE_H

#include <QWidget>

#include "./storebasepage.h"
#include "../widgets/pagination.h"

namespace Ui {
class OrdersPage;
}

class OrdersPage : public StoreBasePage
{
    Q_OBJECT

public:
    explicit OrdersPage(QWidget *parent = nullptr);
    ~OrdersPage();

    virtual void setApiClient(api::GogApiClient *apiClient) override;

private:
    api::GogApiClient *apiClient;
    api::OrderFilter filter;
    unsigned short page;
    Pagination *paginator;
    Ui::OrdersPage *ui;

    void fetchData();

public slots:
    virtual void initialize(const QVariant &data) override;
    virtual void switchUiAuthenticatedState(bool authenticated) override;
private slots:
    void on_searchEdit_textChanged(const QString &arg1);
};

#endif // ORDERSPAGE_H
