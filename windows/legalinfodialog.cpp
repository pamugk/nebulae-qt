#include "legalinfodialog.h"
#include "ui_legalinfodialog.h"

LegalInfoDialog::LegalInfoDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LegalInfoDialog)
{
    ui->setupUi(this);
}

LegalInfoDialog::~LegalInfoDialog()
{
    delete ui;
}
