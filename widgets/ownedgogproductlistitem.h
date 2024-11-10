#ifndef OWNEDGOGPRODUCTLISTITEM_H
#define OWNEDGOGPRODUCTLISTITEM_H

#include <QWidget>

namespace Ui {
class OwnedGogProductListItem;
}

class OwnedGogProductListItem : public QWidget
{
    Q_OBJECT

public:
    explicit OwnedGogProductListItem(QWidget *parent = nullptr);
    ~OwnedGogProductListItem();

    void setCompatible(bool compatible);
    void setCover(const QPixmap &image);
    void setTitle(const QString &title);

signals:
    void clicked();
    void requestedDownloads();
    void requestedInstallation();
    void requestedSettings();
    void requestedStore();
    void requestedSupport();
    void requestedToHide();

protected:
    virtual void enterEvent(QEnterEvent* event) override;
    virtual void leaveEvent(QEvent* event) override;
    virtual void mousePressEvent(QMouseEvent *event) override;

private:
    Ui::OwnedGogProductListItem *ui;
};

#endif // OWNEDGOGPRODUCTLISTITEM_H
