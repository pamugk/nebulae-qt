#ifndef ALLGAMESPAGE_H
#define ALLGAMESPAGE_H

#include <QWidget>

#include "./basepage.h"
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
    api::GogApiClient *apiClient;
    int currentSortOrder;
    QVector<api::SortOrder> orders;
    api::CatalogFilter filter;
    quint16 page;
    Pagination *paginator;
    Ui::AllGamesPage *ui;

    void fetchData();

public slots:
    virtual void clear() override;
    virtual void initialize() override;
private slots:
    void on_lineEdit_textChanged(const QString &arg1);
    void on_sortOrderComboBox_currentIndexChanged(int index);
};

#endif // ALLGAMESPAGE_H
