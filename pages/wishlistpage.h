#ifndef WISHLISTPAGE_H
#define WISHLISTPAGE_H

#include <QWidget>

#include "./storebasepage.h"
#include "../widgets/pagination.h"

namespace Ui {
class WishlistPage;
}

class WishlistPage : public StoreBasePage
{
    Q_OBJECT

public:
    explicit WishlistPage(QWidget *parent = nullptr);
    ~WishlistPage();

    virtual void setApiClient(api::GogApiClient *apiClient) override;

private:
    api::GogApiClient *apiClient;
    QString query;
    QVector<QString> orders;
    unsigned char currentOrder;
    unsigned short page;
    Pagination *paginator;
    Ui::WishlistPage *ui;

    void fetchData();

    Q_SIGNAL void updatingData();

public slots:
    virtual void initialize(const QVariant &data) override;
    virtual void switchUiAuthenticatedState(bool authenticated) override;
private slots:
    void onCurrentOrderChanged(int index);
    void onCurrentVisibilityChanged(int index);
    void onSearchTextChanged(const QString &arg1);
    void on_retryButton_clicked();
};

#endif // WISHLISTPAGE_H
