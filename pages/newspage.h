#ifndef NEWSPAGE_H
#define NEWSPAGE_H

#include <QWidget>

#include "./basepage.h"

namespace Ui {
class NewsPage;
}

class NewsPage : public BasePage
{
    Q_OBJECT

public:
    explicit NewsPage(QWidget *parent = nullptr);
    ~NewsPage();

    virtual void setApiClient(api::GogApiClient *apiClient) override;

private:
    api::GogApiClient *apiClient;
    Ui::NewsPage *ui;

    QNetworkReply *bannerReply;
    QNetworkReply *newsHeadlinesReply;
    QNetworkReply *newsPageReply;

public slots:
    virtual void initialize(const QVariant &data) override;
    virtual void switchUiAuthenticatedState(bool authenticated) override;
};

#endif // NEWSPAGE_H
