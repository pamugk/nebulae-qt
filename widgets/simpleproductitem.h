#ifndef SIMPLEPRODUCTITEM_H
#define SIMPLEPRODUCTITEM_H

#include <QWidget>

namespace Ui {
class SimpleProductItem;
}

class SimpleProductItem : public QWidget
{
    Q_OBJECT

public:
    explicit SimpleProductItem(QWidget *parent = nullptr);
    ~SimpleProductItem();

    void setCover(const QPixmap &image);
    void setDeal(const QDateTime &dealEnd);
    void setOwned(bool owned);
    void setTitle(const QString &title);
    void setPrice(double basePrice, double finalPrice,
                  unsigned char discount, bool free, const QString &currency);
    void setWishlisted(bool wishlisted);

signals:
    void clicked();

public slots:
    void switchUiAuthenticatedState(bool authenticated);

protected:
    void mousePressEvent(QMouseEvent *event) override;

private:
    Ui::SimpleProductItem *ui;
};

#endif // SIMPLEPRODUCTITEM_H
