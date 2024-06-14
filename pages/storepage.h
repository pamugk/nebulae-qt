#ifndef STOREPAGE_H
#define STOREPAGE_H

#include <QWidget>

#include "./storebasepage.h"

namespace Ui {
class StorePage;
}

class StorePage : public StoreBasePage
{
    Q_OBJECT

public:
    explicit StorePage(QWidget *parent = nullptr);
    ~StorePage();

    virtual void setApiClient(api::GogApiClient *apiClient) override;

private:
    api::GogApiClient *apiClient;
    QVector<QString> nowOnSaleSectionIds;
    QVector<bool> nowOnSaleSectionsRequested;
    Ui::StorePage *ui;

    QNetworkReply *customSectionCDPRReply;
    QNetworkReply *customSectionExclusivesReply;
    QNetworkReply *customSectionGOGReply;
    QNetworkReply *dealOfTheDayReply;
    QNetworkReply *discoverBestsellingReply;
    QNetworkReply *discoverNewReply;
    QNetworkReply *discoverUpcomingReply;
    QNetworkReply *newsReply;
    QNetworkReply *nowOnSaleReply;
    QVector<QNetworkReply *> nowOnSaleSectionReplies;

    void getCustomSectionCDPRGames();
    void getCustomSectionExclusiveGames();
    void getCustomSectionGOGGames();
    void getDealOfTheDay();
    void getDiscoverBestsellingGames();
    void getDiscoverNewGames();
    void getDiscoverUpcomingGames();
    void getNews();
    void getNowOnSale();

public slots:
    virtual void initialize(const QVariant &data) override;
    virtual void switchUiAuthenticatedState(bool authenticated) override;
private slots:
    void on_showCatalogButton_clicked();
    void on_nowOnSaleTabWidget_currentChanged(int index);
};

#endif // STOREPAGE_H
