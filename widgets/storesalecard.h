#ifndef STORESALECARD_H
#define STORESALECARD_H

#include <QWidget>

namespace Ui {
class StoreSaleCard;
}

class StoreSaleCard : public QWidget
{
    Q_OBJECT

public:
    explicit StoreSaleCard(QWidget *parent = nullptr);
    ~StoreSaleCard();

    void setBackgroundImage(const QPixmap &image);
    void setColor(std::array<unsigned char, 3> colorRgbArray);
    void setColor(const QString &colorDefinition);
    void setCountdownValue(std::chrono::seconds countdownTimerValue);
    void setDiscount(const QString &discount);
    void setDiscountUpTo(bool upTo);
    void setTitle(const QString &title);

signals:
    void navigateToItem();

protected:
    void mousePressEvent(QMouseEvent *event) override;

private:
    Ui::StoreSaleCard *ui;
};

#endif // STORESALECARD_H
