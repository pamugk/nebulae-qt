#ifndef ACHIEVEMENTLISTITEM_H
#define ACHIEVEMENTLISTITEM_H

#include <QWidget>

#include "../api/gogapiclient.h"
#include "../api/models/platformachievement.h"

namespace Ui {
class AchievementListItem;
}

class AchievementListItem : public QWidget
{
    Q_OBJECT

public:
    explicit AchievementListItem(const api::PlatformAchievement &data,
                                 const QDate &unlockedDate,
                                 api::GogApiClient *apiClient,
                                 QWidget *parent = nullptr);
    ~AchievementListItem();

protected:
    virtual void enterEvent(QEnterEvent* event) override;
    virtual void leaveEvent(QEvent* event) override;

private:
    QNetworkReply *imageReply;
    Ui::AchievementListItem *ui;
};

#endif // ACHIEVEMENTLISTITEM_H
