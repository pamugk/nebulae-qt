#include "bonusitem.h"
#include "ui_bonusitem.h"

#include <QMap>
#include <QSvgWidget>

static QMap<QString, QString> bonusTypeIcons
{
    {"artworks", ":/icons/images.svg"},
    {"avatars", ":/icons/image-portrait.svg"},
    {"audio", ":/icons/music.svg"},
    {"game-add-ons", ":/icons/folder-plus.svg"},
    {"guides-reference", ":/icons/book-bookmark.svg"},
    {"manuals", ":/icons/file-lines.svg"},
    {"video", ":/icons/film.svg"},
    {"wallpapers", ":/icons/display.svg"}
};

BonusItem::BonusItem(const api::Bonus &data, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::BonusItem)
{
    ui->setupUi(this);

    ui->textLabel->setText(data.name);

    if (bonusTypeIcons.contains(data.type.name))
    {
        auto iconWidget = new QSvgWidget(bonusTypeIcons[data.type.name], this);
        iconWidget->setFixedSize(16, 16);
        ui->layout->insertWidget(0, iconWidget);
    }
}

BonusItem::~BonusItem()
{
    delete ui;
}
