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
    void navigateToDestination(NavigationDestination destination);

public slots:
    virtual void clear() = 0;
    virtual void initialize() = 0;
};

#endif // BASEPAGE_H
