#include "allgamespage.h"
#include "ui_allgamespage.h"

AllGamesPage::AllGamesPage(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::AllGamesPage)
{
    ui->setupUi(this);
}

AllGamesPage::~AllGamesPage()
{
    delete ui;
}
