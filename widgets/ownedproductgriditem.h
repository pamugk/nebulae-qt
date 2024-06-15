#ifndef OWNEDPRODUCTGRIDITEM_H
#define OWNEDPRODUCTGRIDITEM_H

#include <QNetworkReply>
#include <QWidget>

#include "../api/gogapiclient.h"
#include "../api/models/ownedproduct.h"

namespace Ui {
class OwnedProductGridItem;
}

class OwnedProductGridItem : public QWidget
{
    Q_OBJECT

public:
    explicit OwnedProductGridItem(const api::OwnedProduct &data,
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
    void setAdditionalDataDisplayed(int kind);
    void setImageSize(const QSize &imageSize);
    void setRatingVisibility(bool visible);
    void setStatusVisibility(bool visible);
    void setTitleVisibility(bool visible);

signals:
    void clicked();

protected:
    void mousePressEvent(QMouseEvent *event) override;

private:
    QString company;
    QString genres;
    QString tags;
    QString title;
    QNetworkReply *imageReply;
    Ui::OwnedProductGridItem *ui;
};

#endif // OWNEDPRODUCTGRIDITEM_H
