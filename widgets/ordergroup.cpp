#include "ordergroup.h"
#include "ui_ordergroup.h"

#include <QDesktopServices>
#include <QLocale>
#include <QMenu>

#include "./orderitem.h"

OrderGroup::OrderGroup(const api::Order &data,
                       api::GogApiClient *apiClient,
                       QWidget *parent) :
    QWidget(parent),
    ui(new Ui::OrderGroup)
{
    ui->setupUi(this);

    auto systemLocale = QLocale::system();
    auto infoMenu = new QMenu(ui->infoButton);
    infoMenu->addAction("VIEW RECEIPT", this, [receiptLink = data.receiptLink]()
    {
        QDesktopServices::openUrl(QString("https://gog.com%1").arg(receiptLink));
    });
    infoMenu->addAction("CONTACT SUPPORT", this, []()
    {
        QDesktopServices::openUrl(QUrl("https://gog.com/support/contact"));
    });
    ui->infoButton->setMenu(infoMenu);

    ui->orderNumberLabel->setText(QString("ORDER #%1").arg(data.publicId.toUpper()));
    if (!data.giftRecipient.isNull())
    {
        ui->giftedLabel->setText(QString("Gifted to %1").arg(data.giftRecipient));
    }
    else if (!data.giftSender.isNull())
    {
        ui->giftedLabel->setText(QString("Gifted by %1").arg(data.giftSender));
    }
    QString purchaseMethod;
    if (data.total.zero)
    {
        purchaseMethod = data.paymentMethod.toUpper();
    }
    else
    {
        purchaseMethod = QString("PAID WITH %1").arg(data.paymentMethod.toUpper());
    }
    ui->priceLabel->setText(QString("%1 (%2)").arg(systemLocale.toCurrencyString(data.total.amount, data.total.symbol), purchaseMethod));
    if (!data.giftCode.isNull())
    {
        ui->giftCodeLabel->setText(QString("Gift code: %1").arg(data.giftCode));
    }
    ui->dateLabel->setText(systemLocale.toString(data.date));

    for (const api::OrderProduct &product : std::as_const(data.products))
    {
        auto productOrderItem = new OrderItem(product, apiClient, this);
        productOrderItem->setCursor(Qt::PointingHandCursor);
        connect(productOrderItem, &OrderItem::clicked, this, [this, productId = product.id]()
        {
           emit navigateToProduct(productId);
        });
        ui->productsLayout->addWidget(productOrderItem);
    }
}

OrderGroup::~OrderGroup()
{
    delete ui;
}

