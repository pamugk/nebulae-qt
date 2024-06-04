#include "storesalebrowseallcard.h"
#include "ui_storesalebrowseallcard.h"

StoreSaleBrowseAllCard::StoreSaleBrowseAllCard(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::StoreSaleBrowseAllCard)
{
    ui->setupUi(this);
}

StoreSaleBrowseAllCard::~StoreSaleBrowseAllCard()
{
    delete ui;
}

void StoreSaleBrowseAllCard::mousePressEvent(QMouseEvent *event)
{
    emit navigateToItem();
}
