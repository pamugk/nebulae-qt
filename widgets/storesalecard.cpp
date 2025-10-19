#include "storesalecard.h"
#include "ui_storesalecard.h"

#include <QNetworkReply>

StoreSaleCard::StoreSaleCard(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::StoreSaleCard)
{
    ui->setupUi(this);
}

StoreSaleCard::~StoreSaleCard()
{
    delete ui;
}

void StoreSaleCard::mousePressEvent(QMouseEvent *event)
{
    emit navigateToItem();
}

void StoreSaleCard::setBackgroundImage(const QPixmap &image)
{
    QPalette backgroundPalette;
    backgroundPalette.setBrush(this->backgroundRole(), QBrush(image.scaled(this->size(), Qt::KeepAspectRatioByExpanding)));
    this->setAutoFillBackground(true);
    this->setPalette(backgroundPalette);
}

void StoreSaleCard::setColor(std::array<unsigned char, 3> colorRgbArray)
{
    ui->content->setStyleSheet(QString("background: rgba(%1, %2, %3, 0.75);")
                               .arg(colorRgbArray[0]).arg(colorRgbArray[1]).arg(colorRgbArray[2]));
}

void StoreSaleCard::setColor(const QString &colorDefinition)
{
    QString usedColor = colorDefinition;
    // TODO: check out reason for such a strange behaviour
    if (colorDefinition.endsWith(QLatin1StringView(",1.00)")))
    {
        usedColor.replace(QLatin1StringView(",1.00)"), QLatin1StringView(",0.75)"));
    }
    ui->content->setStyleSheet(QString("background: %1;").arg(usedColor));
}

void StoreSaleCard::setCountdownValue(std::chrono::seconds countdownTimerValue)
{
    auto days = std::chrono::duration_cast<std::chrono::days>(countdownTimerValue);
    auto hours = std::chrono::duration_cast<std::chrono::hours>(countdownTimerValue - days);
    auto minutes = std::chrono::duration_cast<std::chrono::minutes>(countdownTimerValue - days - hours);
    auto seconds = countdownTimerValue - days - hours - minutes;
    if (days.count() > 0)
    {
        ui->endLabel->setText(tr("%1 d %2h %3m %4s left")
                                  .arg(static_cast<int>(days.count()))
                                  .arg(static_cast<int>(hours.count()), 2, 10, u'0')
                                  .arg(static_cast<int>(minutes.count()), 2, 10, u'0')
                                  .arg(static_cast<int>(seconds.count()), 2, 10, u'0'));;
    }
    else
    {
        ui->endLabel->setText(tr("%1h %2m %3s left")
                                  .arg(static_cast<int>(hours.count()), 2, 10, u'0')
                                  .arg(static_cast<int>(minutes.count()), 2, 10, u'0')
                                  .arg(static_cast<int>(seconds.count()), 2, 10, u'0'));
    }
}

void StoreSaleCard::setDiscount(const QString &discount)
{
    ui->discountLabel->setText(discount);
}

void StoreSaleCard::setTitle(const QString &title)
{
    ui->titleLabel->setText(title);
}

void StoreSaleCard::setDiscountUpTo(bool discountUpTo)
{
    ui->upToLabel->setVisible(discountUpTo);
}
