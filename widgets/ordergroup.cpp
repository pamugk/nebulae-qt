#include "ordergroup.h"
#include "ui_ordergroup.h"

#include "./orderitem.h"

OrderGroup::OrderGroup(const api::Order &data,
                       api::GogApiClient *apiClient,
                       QWidget *parent) :
    QWidget(parent),
    ui(new Ui::OrderGroup)
{
    ui->setupUi(this);

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
    ui->priceLabel->setText(QString("%1 (%2)").arg(data.total.full, purchaseMethod));
    if (!data.giftCode.isNull())
    {
        ui->giftCodeLabel->setText(QString("Gift code: %1").arg(data.giftCode));
    }
    ui->dateLabel->setText(data.date.toString());

    api::OrderProduct product;
    foreach (product, data.products)
    {
        ui->productsLayout->addWidget(new OrderItem(product, apiClient, this));
    }
}

OrderGroup::~OrderGroup()
{
    delete ui;
}

void OrderGroup::on_infoButton_clicked()
{

}

