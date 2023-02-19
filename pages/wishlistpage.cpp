#include "wishlistpage.h"
#include "ui_wishlistpage.h"

#include <QMenu>
#include <QMenuBar>

WishlistPage::WishlistPage(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::WishlistPage)
{
    ui->setupUi(this);

    auto icon = QIcon(":/icons/angle-down.svg");
    auto menuBar = new QMenuBar(this);
    auto genresMenu = menuBar->addMenu("GENRES");
    genresMenu->addAction("SIMULATION");
    genresMenu->addAction("INDIE");
    genresMenu->addAction("ACTION");
    genresMenu->addAction("ADVENTURE");
    genresMenu->addAction("ROLE-PLAYING");
    genresMenu->addAction("RACING");
    genresMenu->addAction("STRATEGY");
    genresMenu->addAction("SHOOTER");
    auto systemMenu = menuBar->addMenu("SYSTEM");
    systemMenu->addAction("Windows");
    systemMenu->addAction("Mac OS");
    systemMenu->addAction("Linux");
    auto languageMenu = menuBar->addMenu("LANGUAGE");
    languageMenu->addAction("ENGLISH");
    auto featureMenu = menuBar->addMenu("FEATURES");
    featureMenu->addAction("SINGLE-PLAYER");
    featureMenu->addAction("ACHIEVEMENTS");
    featureMenu->addAction("CONTROLLER SUPPORT");
    featureMenu->addAction("CLOUD SAVES");
    featureMenu->addAction("OVERLAY");
    featureMenu->addAction("SINGLE-PLAYER");
    featureMenu->addAction("SINGLE-PLAYER");
    auto priceMenu = menuBar->addMenu("PRICE");

    ui->filtersLayout->addWidget(menuBar);
}

WishlistPage::~WishlistPage()
{
    delete ui;
}
