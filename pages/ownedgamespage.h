#ifndef OWNEDGAMESPAGE_H
#define OWNEDGAMESPAGE_H

#include <QWidget>

#include "./basepage.h"

namespace Ui {
class OwnedGamesPage;
}

class OwnedGamesPage : public BasePage
{
    Q_OBJECT

public:
    explicit OwnedGamesPage(QWidget *parent = nullptr);
    ~OwnedGamesPage();

    virtual const QVector<QWidget *> getHeaderControls() override;
    virtual void setApiClient(api::GogApiClient *apiClient) override;

private:
    api::GogApiClient *apiClient;
    QString searchQuery;
    Ui::OwnedGamesPage *ui;
    QVector<QWidget *> uiActions;

    QNetworkReply *ownedGamesReply;

    void updateData();

public slots:
    virtual void initialize(const QVariant &data) override;
    virtual void switchUiAuthenticatedState(bool authenticated) override;
};

#endif // OWNEDGAMESPAGE_H
