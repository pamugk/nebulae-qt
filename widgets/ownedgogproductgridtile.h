#ifndef OWNEDGOGPRODUCTGRIDTILE_H
#define OWNEDGOGPRODUCTGRIDTILE_H

#include <QWidget>

namespace Ui {
class OwnedGogProductGridTile;
}

class OwnedGogProductGridTile : public QWidget
{
    Q_OBJECT

public:
    explicit OwnedGogProductGridTile(QWidget *parent = nullptr);
    ~OwnedGogProductGridTile();

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
    Ui::OwnedGogProductGridTile *ui;
};

#endif // OWNEDGOGPRODUCTGRIDTILE_H
