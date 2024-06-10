#include "collapsiblearea.h"
#include "ui_collapsiblearea.h"

#include <QPropertyAnimation>

#include "./clickablelabel.h"

CollapsibleArea::CollapsibleArea(const QString &header, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CollapsibleArea)
{
    ui->setupUi(this);
    auto titleLabel = new ClickableLabel(this);
    titleLabel->setText(header);
    titleLabel->setCursor(Qt::PointingHandCursor);
    connect(titleLabel, &ClickableLabel::clicked, this, [this]()
    {
        ui->expandButton->setChecked(!ui->expandButton->isChecked());
    });
    ui->labelPlaceholder->addWidget(titleLabel);
    toggleAnimation.addAnimation(new QPropertyAnimation(this, "minimumHeight"));
    toggleAnimation.addAnimation(new QPropertyAnimation(this, "maximumHeight"));
    toggleAnimation.addAnimation(new QPropertyAnimation(ui->scrollArea, "maximumHeight"));

    connect(ui->clearButton, &QPushButton::clicked, this, &CollapsibleArea::clearFilters);
    ui->changedFiltersLabel->setVisible(false);
    ui->clearButton->setVisible(false);
}

CollapsibleArea::~CollapsibleArea()
{
    delete ui;
}

void CollapsibleArea::on_expandButton_toggled(bool checked)
{
    toggleAnimation.setDirection(checked ? QAbstractAnimation::Forward : QAbstractAnimation::Backward);
    toggleAnimation.start();
}

const QLayout *CollapsibleArea::contentLayout()
{
    return ui->scrollArea->layout();
}

void CollapsibleArea::setContentLayout(QLayout *contentLayout)
{
    if (ui->scrollArea->layout() != nullptr)
    {
        ui->scrollArea->layout()->deleteLater();
    }
    ui->scrollArea->setLayout(contentLayout);
    const auto collapsedHeight = sizeHint().height() - ui->scrollArea->maximumHeight();
    auto contentHeight = contentLayout->sizeHint().height();
    for (int i = 0; i < toggleAnimation.animationCount() - 1; ++i)
    {
        QPropertyAnimation * spoilerAnimation = static_cast<QPropertyAnimation *>(toggleAnimation.animationAt(i));
        spoilerAnimation->setDuration(300);
        spoilerAnimation->setStartValue(collapsedHeight);
        spoilerAnimation->setEndValue(collapsedHeight + contentHeight);
    }
    ui->scrollArea->setFixedWidth(contentLayout->sizeHint().width());
    QPropertyAnimation * contentAnimation = static_cast<QPropertyAnimation *>(toggleAnimation.animationAt(toggleAnimation.animationCount() - 1));
    contentAnimation->setDuration(300);
    contentAnimation->setStartValue(0);
    contentAnimation->setEndValue(contentHeight);
}

void CollapsibleArea::setChangedFilters(quint8 changedFilterCount)
{
    auto visibleChange = changedFilterCount > 0;
    ui->changedFiltersLabel->setVisible(visibleChange);
    ui->changedFiltersLabel->setText(QString::number(changedFilterCount));
    ui->clearButton->setVisible(visibleChange);
}

