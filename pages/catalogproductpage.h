#ifndef CATALOGPRODUCTPAGE_H
#define CATALOGPRODUCTPAGE_H

#include "./storebasepage.h"
#include "../api/models/reviewfilters.h"
#include "../api/models/catalogproductinfo.h"

namespace Ui {
class CatalogProductPage;
}

class CatalogProductPage : public StoreBasePage
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

    void on_wishlistButton_clicked();

    void on_wishlistButton_toggled(bool checked);

    void on_libraryButton_clicked();

protected:
    virtual void paintEvent(QPaintEvent *event) override;

private:
    api::GogApiClient *apiClient;
    QPixmap backgroundImage;
    QVector<api::GetCatalogProductInfoResponse> dependentProducts;
    unsigned char dependentProductsLeft;
    QString id;
    QVector<api::GetCatalogProductInfoResponse> requiredProducts;
    unsigned char requiredProductsLeft;
    api::ReviewFilters reviewFilters;
    QVector<api::FormattedLink> screenshots;
    QVector<api::ThumbnailedVideo> videos;
    QSet<const QString> ownedProducts;
    Ui::CatalogProductPage *ui;
    QSet<const QString> wishlist;

    QNetworkReply *lastReviewsReply;

    unsigned short reviewsPage;
    unsigned short reviewsPageSize;
    api::SortOrder reviewsOrder;

    void initializeUserReviewsFilters();
    Q_SIGNAL void ownedProductsChanged(const QSet<const QString> &ids);
    Q_SIGNAL void wishlistChanged(const QSet<const QString> &ids);
};

#endif // CATALOGPRODUCTPAGE_H
