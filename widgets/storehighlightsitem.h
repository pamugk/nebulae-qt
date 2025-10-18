#ifndef STOREHIGHLIGHTSITEM_H
#define STOREHIGHLIGHTSITEM_H

#include <QWidget>

namespace Ui {
class StoreHighlightsItem;
}

class StoreHighlightsItem : public QWidget
{
    Q_OBJECT

public:
    explicit StoreHighlightsItem(QWidget *parent = nullptr);
    ~StoreHighlightsItem();

    void setBackgroundImage(const QPixmap &image);
    void setCustomButton(const QString &text);
    void setLogoImage(const QPixmap &image);
    void setPrice(double basePrice, double finalPrice,
                  unsigned char discount);
    void setSubtitle(const QString &subtitle);
    void setTitle(const QString &title);
    void setWishlisted(bool wishlisted);

signals:
    void clicked();
    void customInfoClicked();

protected:
    void mousePressEvent(QMouseEvent *event) override;

private:
    Ui::StoreHighlightsItem *ui;
};

#endif // STOREHIGHLIGHTSITEM_H
