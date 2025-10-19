#ifndef STOREDYNAMICPAGE_H
#define STOREDYNAMICPAGE_H

#include <QWidget>

#include "./storebasepage.h"

namespace Ui {
class StoreDynamicPage;
}

class StoreDynamicPage : public StoreBasePage
{
    Q_OBJECT

public:
    explicit StoreDynamicPage(QWidget *parent = nullptr);
    ~StoreDynamicPage();

    virtual void setApiClient(api::GogApiClient *apiClient) override;

protected:
    virtual void timerEvent(QTimerEvent *event) override;

private:
    api::GogApiClient *apiClient;
    QSet<const QString> ownedProducts;
    QString pathHex;
    QString promoId;
    std::optional<int> timerId;
    Ui::StoreDynamicPage *ui;
    QSet<const QString> wishlist;

    void getSection(const QString &id, const QString &type);
    void getSections();

    Q_SIGNAL void authenticationStateChanged();
    Q_SIGNAL void ownedProductsChanged(const QSet<const QString> &ids);
    Q_SIGNAL void timeTicked(const QDateTime &currentDateTime);
    Q_SIGNAL void wishlistChanged(const QSet<const QString> &ids);

public slots:
    virtual void initialize(const QVariant &data) override;
    virtual void switchUiAuthenticatedState(bool authenticated) override;
};

#endif // STOREDYNAMICPAGE_H
