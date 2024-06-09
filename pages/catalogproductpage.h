#ifndef CATALOGPRODUCTPAGE_H
#define CATALOGPRODUCTPAGE_H

#include "./basepage.h"
#include "../api/models/reviewfilters.h"
#include "../api/models/catalogproductinfo.h"

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

    void openGalleryOnItem(std::size_t index);

protected:
    virtual void paintEvent(QPaintEvent *event) override;

private:
    api::GogApiClient *apiClient;
    QPixmap backgroundImage;
    QVector<api::GetCatalogProductInfoResponse> dependentProducts;
    unsigned char dependentProductsLeft;
    unsigned long long id;
    QVector<api::GetCatalogProductInfoResponse> requiredProducts;
    unsigned char requiredProductsLeft;
    api::ReviewFilters reviewFilters;
    QVector<api::FormattedLink> screenshots;
    QVector<api::ThumbnailedVideo> videos;
    Ui::CatalogProductPage *ui;

    QNetworkReply *averageRatingReply;
    QNetworkReply *averageOwnerRatingReply;
    QNetworkReply *backgroundReply;
    QVector<QNetworkReply *> dependentProductReplies;
    QNetworkReply *lastReviewsReply;
    QNetworkReply *logotypeReply;
    QNetworkReply *mainReply;
    QNetworkReply *pricesReply;
    QNetworkReply *recommendedPurchasedTogetherReply;
    QNetworkReply *recommendedSimilarReply;
    QVector<QNetworkReply *> requiredProductReplies;
    QNetworkReply *seriesGamesReply;
    QNetworkReply *seriesTotalPriceReply;

    unsigned short reviewsPage;
    unsigned short reviewsPageSize;
    api::SortOrder reviewsOrder;

    void initializeUserReviewsFilters();
};

#endif // CATALOGPRODUCTPAGE_H
