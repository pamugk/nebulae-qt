#ifndef ALLGAMESPAGE_H
#define ALLGAMESPAGE_H

#include <QWidget>

#include "./storebasepage.h"
#include "../api/models/catalog.h"
#include "../widgets/pagination.h"

namespace Ui {
class AllGamesPage;
}

class AllGamesPage : public StoreBasePage
{
    Q_OBJECT

public:
    explicit AllGamesPage(QWidget *parent = nullptr);
    ~AllGamesPage();

    virtual void setApiClient(api::GogApiClient *apiClient) override;

private:
    unsigned short activatedFilterCount;
    api::GogApiClient *apiClient;
    bool applyFilters;
    int currentSortOrder;
    api::SearchCatalogResponse data;
    QVector<api::SortOrder> orders;
    api::CatalogFilter filter;
    bool gridLayout;
    QSet<const QString> ownedProducts;
    unsigned short page;
    Pagination *paginator;
    Ui::AllGamesPage *ui;
    QSet<const QString> wishlist;

    QNetworkReply *lastCatalogReply;
    QNetworkReply *ownedProductsReply;
    QNetworkReply *wishlistReply;

    void fetchData();
    void layoutResults();
    Q_SIGNAL void ownedProductsChanged(const QSet<const QString> &ids);
    Q_SIGNAL void wishlistChanged(const QSet<const QString> &ids);

public slots:
    virtual void initialize(const QVariant &data) override;
    virtual void switchUiAuthenticatedState(bool authenticated) override;
private slots:
    void on_lineEdit_textChanged(const QString &arg1);
    void on_sortOrderComboBox_currentIndexChanged(int index);
    void on_gridModeButton_clicked();
    void on_listModeButton_clicked();
};

#endif // ALLGAMESPAGE_H
