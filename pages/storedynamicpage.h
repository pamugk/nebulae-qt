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
    explicit StoreDynamicPage(const NavigationDestination &destination, QWidget *parent = nullptr);
    ~StoreDynamicPage();

    virtual void setApiClient(api::GogApiClient *apiClient) override;

protected:
    void resizeEvent(QResizeEvent *event) override;
    virtual void timerEvent(QTimerEvent *event) override;

private:
    QMap<const QString, const QWidget *> anchors;
    api::GogApiClient *apiClient;
    QVector<QString> nowOnSaleSectionsIds;
    QVector<bool> nowOnSaleSectionsRequested;
    QSet<const QString> ownedProducts;
    QString path;
    QString pathHex;
    QString promoId;
    std::optional<int> timerId;
    Ui::StoreDynamicPage *ui;
    QSet<const QString> wishlist;

    void getNowOnSale();
    void getSection(const QString &id, const QString &type);
    void getSections();
    void updateWishlistSection(int startIndex, const QString &sectionId);

    Q_SIGNAL void authenticationStateChanged();
    Q_SIGNAL void ownedProductsChanged(const QSet<const QString> &ids);
    Q_SIGNAL void resized(bool widthChanged, bool heightChanged);
    Q_SIGNAL void timeTicked(const QDateTime &currentDateTime);
    Q_SIGNAL void wishlistChanged(const QSet<const QString> &ids);

public slots:
    virtual void initialize(const QVariant &data) override;
    virtual void switchUiAuthenticatedState(bool authenticated) override;
};

#endif // STOREDYNAMICPAGE_H
