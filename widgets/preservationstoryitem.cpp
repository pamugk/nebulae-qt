#include "preservationstoryitem.h"
#include "ui_preservationstoryitem.h"

PreservationStoryItem::PreservationStoryItem(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PreservationStoryItem)
{
    ui->setupUi(this);

    connect(ui->readFullArticleButton, &QPushButton::clicked, this, [this]()
    {
        emit showFullArticleClicked();
    });
}

PreservationStoryItem::~PreservationStoryItem()
{
    delete ui;
}

void PreservationStoryItem::setCover(const QPixmap &cover)
{
    ui->coverLabel->setFixedSize(cover.size());
    ui->coverLabel->setPixmap(cover);
}

void PreservationStoryItem::setDescription(const QString &description)
{
    ui->descriptionLabel->setText(description);
}

void PreservationStoryItem::setReadFullArticleButtonText(const QString &buttonText)
{
    ui->readFullArticleButton->setText(buttonText);
}

void PreservationStoryItem::setTitle(const QString &title)
{
    ui->titleLabel->setText(title);
}
