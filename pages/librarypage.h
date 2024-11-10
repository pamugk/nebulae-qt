#ifndef LIBRARYPAGE_H
#define LIBRARYPAGE_H

#include <QWidget>

#include "./storebasepage.h"
#include "../api/models/ownedproduct.h"
#include "../widgets/pagination.h"

namespace Ui {
class LibraryPage;
}

class LibraryPage : public StoreBasePage
{
    Q_OBJECT

public:
    explicit LibraryPage(QWidget *parent = nullptr);
    ~LibraryPage();

    virtual void setApiClient(api::GogApiClient *apiClient) override;

private:
    api::GogApiClient *apiClient;
    api::GetOwnedProductsResponse data;
    bool gridLayout;
    unsigned short page;
    Pagination *paginator;
    Ui::LibraryPage *ui;

    QNetworkReply *lastLibraryReply;

    void fetchData();
    void layoutResults();
    void openSupport();

public slots:
    virtual void initialize(const QVariant &data) override;
    virtual void switchUiAuthenticatedState(bool authenticated) override;
};

#endif // LIBRARYPAGE_H
