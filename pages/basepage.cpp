#include "./basepage.h"

BasePage::BasePage(QWidget *parent) :
    QWidget(parent)
{
}


BasePage::~BasePage()
{

}

QVector<QWidget *> BasePage::getHeaderControls()
{
    return QVector<QWidget *>();
}
