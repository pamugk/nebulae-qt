#ifndef STOREPAGE_H
#define STOREPAGE_H

#include <QWidget>

namespace Ui {
class StorePage;
}

class StorePage : public QWidget
{
    Q_OBJECT

public:
    explicit StorePage(QWidget *parent = nullptr);
    ~StorePage();

private:
    Ui::StorePage *ui;
};

#endif // STOREPAGE_H
