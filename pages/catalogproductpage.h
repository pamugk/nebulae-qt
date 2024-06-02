#ifndef CATALOGPRODUCTPAGE_H
#define CATALOGPRODUCTPAGE_H

#include "./basepage.h"

namespace Ui {
class CatalogProductPage;
}

class CatalogProductPage : public BasePage
{
    Q_OBJECT

public:
    explicit CatalogProductPage(QWidget *parent = nullptr);
    ~CatalogProductPage();

    virtual void setApiClient(api::GogApiClient *apiClient) override;

public slots:
    virtual void initialize(const QVariant &data) override;
    virtual void switchUiAuthenticatedState(bool authenticated) override;

signals:
    void userReviewsResultsUpdated(unsigned short page, unsigned short totalPages);

private slots:
    void on_acceptContentWarningButton_clicked();

    void on_goBackButton_clicked();

    void descriptionViewContentsSizeChanged(const QSizeF &size);

    void updateUserReviews();

    void changeUserReviewsPage(unsigned short page);

    void on_userReviewsPageSizeComboBox_currentIndexChanged(int index);

    void on_userReviewsSortOrderComboBox_currentIndexChanged(int index);

private:
    api::GogApiClient *apiClient;
    unsigned long long id;
    Ui::CatalogProductPage *ui;

    QNetworkReply *averageRatingReply;
    QNetworkReply *averageOwnerRatingReply;
    QNetworkReply *lastReviewsReply;
    QNetworkReply *mainReply;
    QNetworkReply *pricesReply;
    QNetworkReply *recommendedPurchasedTogetherReply;
    QNetworkReply *recommendedSimilarReply;
    QNetworkReply *seriesGamesReply;
    QNetworkReply *seriesTotalPriceReply;

    unsigned short reviewsPage;
    unsigned short reviewsPageSize;
    api::SortOrder reviewsOrder;
};

#endif // CATALOGPRODUCTPAGE_H
