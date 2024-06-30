#ifndef DISCOVERPAGE_H
#define DISCOVERPAGE_H

#include <QWidget>

#include "basepage.h"

namespace Ui {
class DiscoverPage;
}

class DiscoverPage : public BasePage
{
    Q_OBJECT

public:
    explicit DiscoverPage(QWidget *parent = nullptr);
    ~DiscoverPage();

    virtual void setApiClient(api::GogApiClient *apiClient) override;

private:
    api::GogApiClient *apiClient;
    Ui::DiscoverPage *ui;

public slots:
    virtual void initialize(const QVariant &data) override;
    virtual void switchUiAuthenticatedState(bool authenticated) override;
};

#endif // DISCOVERPAGE_H
