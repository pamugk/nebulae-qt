#include "downloadlistitem.h"
#include "ui_downloadlistitem.h"

DownloadListItem::DownloadListItem(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::DownloadListItem)
{
    ui->setupUi(this);

    ui->sizeLabel->setVisible(false);
    ui->versionLabel->setVisible(false);
}

DownloadListItem::~DownloadListItem()
{
    delete ui;
}

void DownloadListItem::mousePressEvent(QMouseEvent* event)
{
    emit clicked();
}

void DownloadListItem::setSize(const QString &value)
{
    ui->sizeLabel->setText(value);
    ui->sizeLabel->setVisible(true);
}

void DownloadListItem::setTitle(const QString &value)
{
    ui->titleLabel->setText(value);
    ui->titleLabel->setVisible(true);
}

void DownloadListItem::setVersion(const QString &value)
{
    ui->versionLabel->setText(value);
    ui->versionLabel->setVisible(true);
}
