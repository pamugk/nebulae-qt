#ifndef STOREBASEPAGE_H
#define STOREBASEPAGE_H

#include "basepage.h"

class StoreBasePage : public BasePage
{
public:
    StoreBasePage(const NavigationDestination &destination, QWidget *parent = nullptr);
    ~StoreBasePage();

    virtual const QVector<QWidget *> getHeaderControls() override;

public slots:
    virtual void switchUiAuthenticatedState(bool authenticated) override;

private:
    QVector<QWidget *> actions;
};

#endif // STOREBASEPAGE_H
