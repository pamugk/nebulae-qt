#ifndef STORESALEBROWSEALLCARD_H
#define STORESALEBROWSEALLCARD_H

#include <QWidget>

namespace Ui {
class StoreSaleBrowseAllCard;
}

class StoreSaleBrowseAllCard : public QWidget
{
    Q_OBJECT

public:
    explicit StoreSaleBrowseAllCard(QWidget *parent = nullptr);
    ~StoreSaleBrowseAllCard();

signals:
    void navigateToItem();

protected:
    void mousePressEvent(QMouseEvent *event) override;

private:
    Ui::StoreSaleBrowseAllCard *ui;
};

#endif // STORESALEBROWSEALLCARD_H
