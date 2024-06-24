#ifndef OWNEDGAMESPAGE_H
#define OWNEDGAMESPAGE_H

#include <QWidget>

#include "./basepage.h"
#include "../api/models/userlibraryrequest.h"
#include "../db/userreleasedetails.h"

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
    int currentGridImageSize;
    int currentListIconSize;
    QVector<db::UserReleaseGroup> data;
    const std::array<const QSize, 6> gridImageSizes;
    bool gridLayout;
    QMap<QString, QNetworkReply *> listIconReplies;
    const std::array<const QSize, 3> listIconSizes;
    api::SearchUserReleasesRequest request;
    Ui::OwnedGamesPage *ui;
    QVector<QWidget *> uiActions;

    Q_SIGNAL void gridItemAdditionalDataVisibilityChanged(bool visible);
    Q_SIGNAL void gridItemAdditionalDataDisplayed(int kind);
    Q_SIGNAL void gridItemImageSizeChanged(const QSize &imageSize);
    Q_SIGNAL void gridItemRatingVisibilityChanged(bool visible);
    Q_SIGNAL void gridItemStatusVisibilityChanged(bool visible);
    Q_SIGNAL void gridItemTitleVisibilityChanged(bool visible);
    Q_SIGNAL void listItemReceivedIcon(const QString &releaseId, const QPixmap &icon);

    void layoutData();
    void updateData();
    void updateFilters();

public slots:
    virtual void initialize(const QVariant &data) override;
    virtual void switchUiAuthenticatedState(bool authenticated) override;

private slots:
    void on_retryButton_clicked();
};

#endif // OWNEDGAMESPAGE_H
