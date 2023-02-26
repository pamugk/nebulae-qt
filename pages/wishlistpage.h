#ifndef WISHLISTPAGE_H
#define WISHLISTPAGE_H

#include <QWidget>

#include "./basepage.h"
#include "../widgets/pagination.h"

namespace Ui {
class WishlistPage;
}

class WishlistPage : public BasePage
{

public:
    explicit WishlistPage(QWidget *parent = nullptr);
    ~WishlistPage();

    virtual void setApiClient(api::GogApiClient *apiClient) override;

private:
    api::GogApiClient *apiClient;
    QString query;
    QVector<QString> orders;
    quint8 currentOrder;
    quint16 page;
    Pagination *paginator;
    Ui::WishlistPage *ui;

    void fetchData();

public slots:
    virtual void clear() override;
    virtual void initialize() override;
private slots:
    void onSearchTextChanged(const QString &arg1);
    void onCurrentOrderChanged(int index);
};

#endif // WISHLISTPAGE_H
