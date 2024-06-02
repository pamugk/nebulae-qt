#ifndef BASEPAGE_H
#define BASEPAGE_H

#include <QWidget>

#include "../api/gogapiclient.h"
#include "../internals/navigationdestination.h"

class BasePage : public QWidget
{
    Q_OBJECT

public:
    explicit BasePage(QWidget *parent = nullptr);
    ~BasePage();

    virtual void setApiClient(api::GogApiClient *apiClient) = 0;

signals:
    void navigateBack();
    void navigate(NavigationDestination destination);

public slots:
    virtual void switchUiAuthenticatedState(bool authenticated) = 0;
    virtual void initialize(const QVariant &data) = 0;
};

#endif // BASEPAGE_H
