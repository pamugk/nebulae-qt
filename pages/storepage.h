#ifndef STOREPAGE_H
#define STOREPAGE_H

#include <QWidget>

#include "./basepage.h"

namespace Ui {
class StorePage;
}

class StorePage : public BasePage
{
    Q_OBJECT

public:
    explicit StorePage(QWidget *parent = nullptr);
    ~StorePage();

    virtual void setApiClient(api::GogApiClient *apiClient) override;

private:
    Ui::StorePage *ui;

public slots:
    virtual void clear() override;
    virtual void initialize() override;
};

#endif // STOREPAGE_H
