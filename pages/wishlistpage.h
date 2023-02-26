#ifndef WISHLISTPAGE_H
#define WISHLISTPAGE_H

#include <QWidget>

#include "./basepage.h"

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
    Ui::WishlistPage *ui;

    void fetchData();

public slots:
    virtual void clear() override;
    virtual void initialize() override;
private slots:
    void on_searchEdit_textChanged(const QString &arg1);
};

#endif // WISHLISTPAGE_H
