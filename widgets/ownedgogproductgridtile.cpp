#include "ownedgogproductgridtile.h"
#include "ui_ownedgogproductgridtile.h"

#include <QMenu>
#include <QMouseEvent>

OwnedGogProductGridTile::OwnedGogProductGridTile(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::OwnedGogProductGridTile)
{
    ui->setupUi(this);
    ui->tagsLabel->setVisible(false);

    auto productMenu = new QMenu(ui->menuButton);
    productMenu->addAction("INSTALL", this, [this]()
    {
        emit requestedInstallation();
    });
    productMenu->addAction("SETTINGS", this, [this]()
    {
        emit requestedSettings();
    });
    productMenu->addAction("BACKUPS & GOODIES", this, [this]()
    {
        emit requestedDownloads();
    });
    productMenu->addAction("ASSIGN TAGS…");
    productMenu->addAction("STORE PAGE & REVIEWS", this, [this]()
    {
        emit requestedStore();
    });
    productMenu->addAction("SUPPORT", this, [this]()
    {
        emit requestedSupport();
    });
    productMenu->addAction("HIDE GAME", this, [this]()
    {
        emit requestedToHide();
    });
    ui->menuButton->setMenu(productMenu);
    ui->menuButton->setVisible(false);
}

OwnedGogProductGridTile::~OwnedGogProductGridTile()
{
    delete ui;
}

void OwnedGogProductGridTile::enterEvent(QEnterEvent* event)
{
    ui->menuButton->setVisible(true);
}

void OwnedGogProductGridTile::leaveEvent(QEvent* event)
{
    ui->menuButton->setVisible(false);
}

void OwnedGogProductGridTile::mousePressEvent(QMouseEvent *event)
{
    emit clicked();
}

void OwnedGogProductGridTile::setCompatible(bool compatible)
{

}

void OwnedGogProductGridTile::setCover(const QPixmap &image)
{
    ui->coverLabel->setPixmap(image.scaled(ui->coverLabel->size()));
}

void OwnedGogProductGridTile::setTitle(const QString &title)
{
    ui->titleLabel->setText(title);
}
