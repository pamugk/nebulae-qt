#ifndef ALLGAMESPAGE_H
#define ALLGAMESPAGE_H

#include <QWidget>

#include "./basepage.h"

namespace Ui {
class AllGamesPage;
}

class AllGamesPage : public BasePage
{
    Q_OBJECT

public:
    explicit AllGamesPage(QWidget *parent = nullptr);
    ~AllGamesPage();

    virtual void setApiClient(GogApiClient *apiClient) override;

private:
    Ui::AllGamesPage *ui;

public slots:
    virtual void clear() override;
    virtual void initialize() override;
};

#endif // ALLGAMESPAGE_H
