#ifndef OWNEDPRODUCTGRIDITEM_H
#define OWNEDPRODUCTGRIDITEM_H

#include <QNetworkReply>
#include <QWidget>

#include "../api/gogapiclient.h"
#include "../db/userreleasedetails.h"

namespace Ui {
class OwnedProductGridItem;
}

class OwnedProductGridItem : public QWidget
{
    Q_OBJECT

public:
    explicit OwnedProductGridItem(const db::UserReleaseShortDetails &data,
                                  api::GogApiClient *apiClient,
                                  QWidget *parent = nullptr);
    ~OwnedProductGridItem();

    enum AdditionalInfo
    {
        COMPANY,
        GENRES,
        STATS,
        TAGS,
        PLATFORM
    };

public slots:
    void setAdditionalDataVisibility(bool visible);
    void setAdditionalDataDisplayed(AdditionalInfo kind);
    void setImageSize(const QSize &imageSize);
    void setRatingVisibility(bool visible);
    void setStatusVisibility(bool visible);
    void setTitleVisibility(bool visible);

signals:
    void clicked();

protected:
    void mousePressEvent(QMouseEvent *event) override;

private:
    const QMap<AdditionalInfo, QString> additionalData;
    AdditionalInfo displayedAdditionalData;
    const std::optional<double> rating;
    const QString title;
    QNetworkReply *imageReply;
    Ui::OwnedProductGridItem *ui;
};

#endif // OWNEDPRODUCTGRIDITEM_H
