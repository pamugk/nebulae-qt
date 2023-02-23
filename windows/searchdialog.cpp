#include "searchdialog.h"
#include "ui_searchdialog.h"
#include "../viewmodels/searchlistmodel.h"

SearchDialog::SearchDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SearchDialog)
{
    ui->setupUi(this);
    ui->stackedWidget->setCurrentWidget(ui->startPage);
    ui->resultsListView->setModel(new SearchListModel(ui->resultsListView));
    this->setFixedSize(600, 240);
}

SearchDialog::~SearchDialog()
{
    delete ui;
}

void SearchDialog::on_searchButton_clicked()
{
    ui->stackedWidget->setCurrentWidget(ui->resultsPage);
    this->setFixedHeight(532);
}

