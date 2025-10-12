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
    QVector<QString> nowOnSaleSectionsIds;
    QVector<bool> nowOnSaleSectionsRequested;
    QSet<const QString> ownedProducts;
    Ui::StorePage *ui;
    QSet<const QString> wishlist;

    void getNowOnSale();
    void getSection(const QString &id, const QString &type);
    void getSections();

    Q_SIGNAL void authenticationStateChanged();
    Q_SIGNAL void ownedProductsChanged(const QSet<const QString> &ids);
    Q_SIGNAL void wishlistChanged(const QSet<const QString> &ids);

public slots:
    virtual void initialize(const QVariant &data) override;
    virtual void switchUiAuthenticatedState(bool authenticated) override;
};

#endif // STOREPAGE_H
