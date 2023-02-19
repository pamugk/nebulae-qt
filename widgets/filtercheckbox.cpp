#include "filtercheckbox.h"
#include "ui_filtercheckbox.h"

FilterCheckbox::FilterCheckbox(const QString &text, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FilterCheckbox)
{
    ui->setupUi(this);
    ui->filterCheckbox->setText(text);
}

FilterCheckbox::~FilterCheckbox()
{
    delete ui;
}

void FilterCheckbox::on_hideButton_toggled(bool checked)
{
    ui->filterCheckbox->setEnabled(checked);
    if (!checked) {
        ui->filterCheckbox->setCheckState(Qt::CheckState::Unchecked);
    }
}


void FilterCheckbox::on_filterCheckbox_stateChanged(int arg1)
{

}

