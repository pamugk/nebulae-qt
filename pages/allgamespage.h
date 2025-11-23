#ifndef ALLGAMESPAGE_H
#define ALLGAMESPAGE_H

#include <QWidget>

#include "./storebasepage.h"

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
    api::GogApiClient *apiClient;
    Ui::AllGamesPage *ui;

    Q_SIGNAL void authenticationStateChanged();
    Q_SIGNAL void ownedProductsChanged(const QSet<const QString> &ids);
    Q_SIGNAL void wishlistChanged(const QSet<const QString> &ids);

public slots:
    virtual void initialize(const QVariant &data) override;
    virtual void switchUiAuthenticatedState(bool authenticated) override;
};

#endif // ALLGAMESPAGE_H
