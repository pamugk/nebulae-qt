#include "checkeditem.h"
#include "ui_checkeditem.h"

CheckedItem::CheckedItem(const QString &text, bool checked, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CheckedItem)
{
    ui->setupUi(this);

    ui->checkmarkLabel->setEnabled(checked);
    ui->textLabel->setEnabled(checked);
    ui->textLabel->setText(text);
}

CheckedItem::~CheckedItem()
{
    delete ui;
}
