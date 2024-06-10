#include "clearfilterbutton.h"
#include "ui_clearfilterbutton.h"

ClearFilterButton::ClearFilterButton(const QString &sectionName,
                                     const QString &name,
                                     QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ClearFilterButton)
{
    ui->setupUi(this);
    ui->sectionLabel->setText(sectionName);
    ui->sectionLabel->setVisible(!sectionName.isEmpty());
    ui->nameLabel->setText(name);
    ui->nameLabel->setVisible(!name.isEmpty());
}

ClearFilterButton::~ClearFilterButton()
{
    delete ui;
}

void ClearFilterButton::enterEvent(QEnterEvent* event)
{
    QFont font;
    font.setBold(true);
    font.setStrikeOut(true);

    ui->sectionLabel->setFont(font);
    ui->nameLabel->setFont(font);
}

void ClearFilterButton::leaveEvent(QEvent* event)
{
    QFont font;
    font.setBold(true);

    ui->sectionLabel->setFont(font);
    ui->nameLabel->setFont(font);
}

void ClearFilterButton::mousePressEvent(QMouseEvent* event) {
    emit clicked();
}
