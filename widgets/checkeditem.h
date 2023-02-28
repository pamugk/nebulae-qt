#ifndef CHECKEDITEM_H
#define CHECKEDITEM_H

#include <QWidget>

namespace Ui {
class CheckedItem;
}

class CheckedItem : public QWidget
{
    Q_OBJECT

public:
    explicit CheckedItem(const QString &text, bool checked, QWidget *parent = nullptr);
    ~CheckedItem();

private:
    Ui::CheckedItem *ui;
};

#endif // CHECKEDITEM_H
