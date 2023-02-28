#include "featureitem.h"
#include "ui_featureitem.h"

#include <QSvgWidget>

FeatureItem::FeatureItem(const api::MetaTag &feature, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FeatureItem)
{
    ui->setupUi(this);
    ui->textLabel->setText(feature.name);

    QString icon;
    if (feature.slug == "single")
    {
        icon = ":/icons/user.svg";
    }
    else if(feature.slug == "multi")
    {
        icon = ":/icons/people-group.svg";
    }
    else if(feature.slug == "coop")
    {
        icon = ":/icons/user-group.svg";
    }
    else if(feature.slug == "achievements")
    {
        icon = ":/icons/trophy.svg";
    }
    else if(feature.slug == "leaderboards")
    {
        icon = ":/icons/crown.svg";
    }
    else if(feature.slug == "controller_support")
    {
        icon = ":/icons/gamepad.svg";
    }
    else if(feature.slug == "cloud_saves")
    {
        icon = ":/icons/cloud-arrow-up.svg";
    }
    else if(feature.slug == "overlay")
    {
        icon = ":/icons/object-group.svg";
    }

    if (!icon.isEmpty())
    {
        auto iconWidget = new QSvgWidget(icon, this);
        iconWidget->setFixedSize(16, 16);
        ui->layout->insertWidget(0, iconWidget);
    }
}

FeatureItem::~FeatureItem()
{
    delete ui;
}
