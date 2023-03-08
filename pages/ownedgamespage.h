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

    virtual void setApiClient(api::GogApiClient *apiClient) override;

private:
    api::GogApiClient *apiClient;
    QString searchQuery;
    Ui::OwnedGamesPage *ui;

    void updateData();

public slots:
    virtual void clear() override;
    virtual void initialize() override;
private slots:
    void on_searchLineEdit_textChanged(const QString &arg1);
};

#endif // OWNEDGAMESPAGE_H
