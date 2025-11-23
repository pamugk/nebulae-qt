#ifndef STORECATALOGSECTION_H
#define STORECATALOGSECTION_H

#include <QWidget>

#include "../api/gogapiclient.h"
#include "../api/models/catalog.h"
#include "../internals/navigationdestination.h"
#include "../widgets/pagination.h"

namespace Ui {
class StoreCatalogSection;
}

class StoreCatalogSection : public QWidget
{
    Q_OBJECT

public:
    explicit StoreCatalogSection(QWidget *parent = nullptr);
    ~StoreCatalogSection();

    void initialize(const QVariant &data, api::GogApiClient *apiClient, bool externalContentScroll);

signals:
    void navigate(NavigationDestination destination);

public slots:
    void updateOwnedProducts(const QSet<const QString> &ids);
    void updateWishlist(const QSet<const QString> &ids);

private:
    unsigned short activatedFilterCount;
    api::GogApiClient *apiClient;
    bool applyFilters;
    int currentSortOrder;
    api::SearchCatalogResponse data;
    QVector<api::SortOrder> orders;
    api::CatalogFilter filter;
    bool gridLayout;
    QWidget *gridResultsPage;
    QWidget *listResultsPage;
    QSet<const QString> ownedProducts;
    unsigned short page;
    Pagination *paginator;
    Ui::StoreCatalogSection *ui;
    QSet<const QString> wishlist;

    void fetchData();
    void layoutResults();

    Q_SIGNAL void ownedProductsChanged(const QSet<const QString> &ids);
    Q_SIGNAL void updatingData();
    Q_SIGNAL void wishlistChanged(const QSet<const QString> &ids);
};

#endif // STORECATALOGSECTION_H
