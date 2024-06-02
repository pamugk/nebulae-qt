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
    api::GogApiClient *apiClient;
    Ui::StorePage *ui;

    QNetworkReply *discoverBestsellingReply;
    QNetworkReply *discoverNewReply;
    QNetworkReply *discoverUpcomingReply;
    QNetworkReply *newsReply;

    void getDiscoverBestsellingGames();
    void getDiscoverNewGames();
    void getDiscoverUpcomingGames();
    void getNews();

public slots:
    virtual void initialize(const QVariant &data) override;
    virtual void switchUiAuthenticatedState(bool authenticated) override;
private slots:
    void on_showCatalogButton_clicked();
};

#endif // STOREPAGE_H
