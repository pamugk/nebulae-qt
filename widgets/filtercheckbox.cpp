#include "filtercheckbox.h"
#include "ui_filtercheckbox.h"

FilterCheckbox::FilterCheckbox(const QString &text, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FilterCheckbox)
{
    ui->setupUi(this);
    ui->filterCheckbox->setText(text);
    connect(ui->filterCheckbox, &QCheckBox::toggled, this, &FilterCheckbox::include);
    connect(ui->hideButton, &QAbstractButton::toggled, this, &FilterCheckbox::exclude);
}

FilterCheckbox::~FilterCheckbox()
{
    delete ui;
}

void FilterCheckbox::on_hideButton_toggled(bool checked)
{
    ui->filterCheckbox->setEnabled(!checked);
    if (checked) {
        ui->filterCheckbox->setCheckState(Qt::CheckState::Unchecked);
    }
}

