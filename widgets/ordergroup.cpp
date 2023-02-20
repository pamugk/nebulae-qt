#include "ordergroup.h"
#include "ui_ordergroup.h"

OrderGroup::OrderGroup(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::OrderGroup)
{
    ui->setupUi(this);
}

OrderGroup::~OrderGroup()
{
    delete ui;
}
