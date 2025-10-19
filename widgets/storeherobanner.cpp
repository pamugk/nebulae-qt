#include "storeherobanner.h"
#include "ui_storeherobanner.h"

#include <QDateTime>
#include <QPainter>

StoreHeroBanner::StoreHeroBanner(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::StoreHeroBanner)
{
    ui->setupUi(this);

    ui->countdownLabel->setVisible(false);
    connect(ui->primaryButton, &QPushButton::clicked, this, [this]()
    {
        emit primaryButtonClicked();
    });
}

StoreHeroBanner::~StoreHeroBanner()
{
    delete ui;
}

void StoreHeroBanner::setBackgroundImage(const QByteArray &data)
{
    backgroundImage.loadFromData(data);
    setFixedHeight(backgroundImage.height());
}

void StoreHeroBanner::setCountdownValue(std::chrono::seconds countdownTimerValue)
{
    auto days = std::chrono::duration_cast<std::chrono::days>(countdownTimerValue);
    auto hours = std::chrono::duration_cast<std::chrono::hours>(countdownTimerValue - days);
    auto minutes = std::chrono::duration_cast<std::chrono::minutes>(countdownTimerValue - days - hours);
    auto seconds = countdownTimerValue - days - hours - minutes;
    if (days.count() > 0)
    {
        ui->countdownLabel->setText(tr("%1 days %2 hours %3 minutes %4 seconds left")
                                  .arg(static_cast<int>(days.count()))
                                  .arg(static_cast<int>(hours.count()), 2, 10, u'0')
                                  .arg(static_cast<int>(minutes.count()), 2, 10, u'0')
                                  .arg(static_cast<int>(seconds.count()), 2, 10, u'0'));;
    }
    else
    {
        ui->countdownLabel->setText(tr("%1 hours %2 minutes %3 seconds left")
                                  .arg(static_cast<int>(hours.count()), 2, 10, u'0')
                                  .arg(static_cast<int>(minutes.count()), 2, 10, u'0')
                                  .arg(static_cast<int>(seconds.count()), 2, 10, u'0'));
    }
    ui->countdownLabel->setVisible(true);
}

void StoreHeroBanner::setDescription(const QString &description)
{
    ui->descriptionLabel->setText(description);
    ui->descriptionLabel->setVisible(!description.isEmpty());
}

void StoreHeroBanner::setPrimaryButtonText(const QString &primaryButtonText)
{
    ui->primaryButton->setText(primaryButtonText);
    ui->primaryButton->setVisible(!primaryButtonText.isEmpty());
    ui->primaryButton->setFixedSize(ui->primaryButton->sizeHint());
}

void StoreHeroBanner::setTitle(const QString &title)
{
    ui->titleLabel->setText(title);
    ui->titleLabel->setVisible(!title.isEmpty());
}

void StoreHeroBanner::setUseDarkTheme(bool useDarkTheme)
{
    ui->contentWidget->setStyleSheet(QLatin1StringView(useDarkTheme ? "background-color: none; color: rgb(33, 33, 33);" : "background-color: none; color: rgb(242, 242, 242);"));
}

void StoreHeroBanner::paintEvent(QPaintEvent *event)
{
    if (!backgroundImage.isNull())
    {
        QPainter backgroundPainter(this);
        int x, sx, width;
        if (backgroundImage.width() > this->width())
        {
            x = 0;
            sx = (backgroundImage.width() - this->width()) / 2;
            width = this->width();
        }
        else
        {
            x = (backgroundImage.width() - this->width()) / 2;
            sx = 0;
            width = backgroundImage.width();
        }
        backgroundPainter.drawPixmap(x, 0, backgroundImage,
                                     sx, 0,
                                     width, backgroundImage.height());
    }

    QWidget::paintEvent(event);
}
