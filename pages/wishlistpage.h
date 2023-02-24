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

    virtual void setApiClient(GogApiClient *apiClient) override;

private:
    GogApiClient *apiClient;
    Ui::WishlistPage *ui;

public slots:
    virtual void clear() override;
    virtual void initialize() override;
};

#endif // WISHLISTPAGE_H
