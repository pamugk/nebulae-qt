#ifndef ALLGAMESPAGE_H
#define ALLGAMESPAGE_H

#include <QWidget>

#include "./basepage.h"
#include "../api/models/catalog.h"
#include "../widgets/pagination.h"

namespace Ui {
class AllGamesPage;
}

class AllGamesPage : public BasePage
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
    unsigned short page;
    Pagination *paginator;
    Ui::AllGamesPage *ui;

    QNetworkReply *lastCatalogReply;

    void fetchData();
    void layoutResults();

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
