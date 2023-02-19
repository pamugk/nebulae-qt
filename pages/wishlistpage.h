#ifndef WISHLISTPAGE_H
#define WISHLISTPAGE_H

#include <QWidget>

namespace Ui {
class WishlistPage;
}

class WishlistPage : public QWidget
{
    Q_OBJECT

public:
    explicit WishlistPage(QWidget *parent = nullptr);
    ~WishlistPage();

private:
    Ui::WishlistPage *ui;
};

#endif // WISHLISTPAGE_H
