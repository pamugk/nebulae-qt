#include "ownedgogproductlistitem.h"
#include "ui_ownedgogproductlistitem.h"

#include <QMenu>
#include <QMouseEvent>

OwnedGogProductListItem::OwnedGogProductListItem(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::OwnedGogProductListItem)
{
    ui->setupUi(this);

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

OwnedGogProductListItem::~OwnedGogProductListItem()
{
    delete ui;
}

void OwnedGogProductListItem::enterEvent(QEnterEvent* event)
{
    ui->menuButton->setVisible(true);
}

void OwnedGogProductListItem::leaveEvent(QEvent* event)
{
    ui->menuButton->setVisible(false);
}

void OwnedGogProductListItem::mousePressEvent(QMouseEvent *event)
{
    emit clicked();
}

void OwnedGogProductListItem::setCompatible(bool compatible)
{

}

void OwnedGogProductListItem::setCover(const QPixmap &image)
{
    ui->coverLabel->setPixmap(image.scaled(ui->coverLabel->size()));
}

void OwnedGogProductListItem::setTitle(const QString &title)
{
    ui->titleLabel->setText(title);
}
