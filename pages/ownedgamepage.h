#ifndef OWNEDGAMEPAGE_H
#define OWNEDGAMEPAGE_H

#include <QWidget>

#include "./basepage.h"

namespace Ui {
class OwnedGamePage;
}

class OwnedGamePage : public BasePage
{
    Q_OBJECT

public:
    explicit OwnedGamePage(QWidget *parent = nullptr);
    ~OwnedGamePage();

    virtual void setApiClient(api::GogApiClient *apiClient) override;

private:
    api::GogApiClient *apiClient;
    Ui::OwnedGamePage *ui;

public slots:
    virtual void clear() override;
    virtual void initialize(const QVariant &data) override;
private slots:
    void on_expandDescriptionButton_clicked();
};

#endif // OWNEDGAMEPAGE_H
