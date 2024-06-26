#ifndef RELEASEPAGE_H
#define RELEASEPAGE_H

#include <QWidget>

#include "./basepage.h"
#include "../api/models/game.h"

namespace Ui {
class ReleasePage;
}

class ReleasePage : public BasePage
{
    Q_OBJECT

public:
    explicit ReleasePage(QWidget *parent = nullptr);
    ~ReleasePage();

    virtual const QVector<QWidget *> getHeaderControls() override;
    virtual void setApiClient(api::GogApiClient *apiClient) override;

private:
    api::GogApiClient *apiClient;
    QVector<QString> screenshots;
    QVector<api::NamedVideo> videos;
    Ui::ReleasePage *ui;
    QVector<QWidget *> uiActions;

    QNetworkReply *releaseAchievementsReply;
    QNetworkReply *releaseGametimeStatisticsReply;
    QNetworkReply *releaseReply;

public slots:
    virtual void initialize(const QVariant &data) override;
    virtual void switchUiAuthenticatedState(bool authenticated) override;

private slots:
    void openGalleryOnItem(std::size_t index);
};

#endif // RELEASEPAGE_H
