#ifndef STOREGRIDTILE_H
#define STOREGRIDTILE_H

#include <QWidget>

namespace Ui {
class StoreGridTile;
}

class StoreGridTile : public QWidget
{
    Q_OBJECT

public:
    explicit StoreGridTile(QWidget *parent = nullptr);
    ~StoreGridTile();

private:
    Ui::StoreGridTile *ui;
};

#endif // STOREGRIDTILE_H
