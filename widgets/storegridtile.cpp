#include "storegridtile.h"
#include "ui_storegridtile.h"

StoreGridTile::StoreGridTile(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::StoreGridTile)
{
    ui->setupUi(this);
}

StoreGridTile::~StoreGridTile()
{
    delete ui;
}
