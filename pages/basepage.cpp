#include "./basepage.h"

BasePage::BasePage(QWidget *parent) :
    QWidget(parent)
{
}


BasePage::~BasePage()
{

}

const QVector<QWidget *> BasePage::getHeaderControls()
{
    return QVector<QWidget *>();
}
