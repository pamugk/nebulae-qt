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
    virtual void clear() override;
    virtual void initialize(const QVariant &data) override;

signals:
    void userReviewsResultsUpdated(quint16 page, quint16 totalPages);

private slots:
    void on_acceptContentWarningButton_clicked();

    void on_goBackButton_clicked();

    void descriptionViewContentsSizeChanged(const QSizeF &size);

    void updateUserReviews();

    void changeUserReviewsPage(quint16 page);

    void on_userReviewsPageSizeComboBox_currentIndexChanged(int index);

    void on_userReviewsSortOrderComboBox_currentIndexChanged(int index);

private:
    api::GogApiClient *apiClient;
    quint64 id;
    Ui::CatalogProductPage *ui;

    quint16 reviewsPage;
    quint16 reviewsPageSize;
    api::SortOrder reviewsOrder;
};

#endif // CATALOGPRODUCTPAGE_H
