#include "./orderserialization.h"

#include <QJsonArray>

void parseProductPrice(const QJsonValue &json, api::OrderProductPrice &data)
{
    data.baseAmount = json["baseAmount"].toString().toDouble();
    data.amount = json["amount"].toString().toDouble();
    data.free = json["isFree"].toBool();
    data.discounted = json["isDiscounted"].toBool();
    data.symbol = json["symbol"].toString();
}

void parseOrderPrice(const QJsonValue &json, api::OrderPrice &data)
{
    data.amount = json["amount"].toString().toDouble();
    data.symbol = json["symbol"].toString();
    data.code = json["code"].toString();
    data.zero = json["isZero"].toBool();
    data.rawAmount = json["rawAmount"].toInt();
    data.formattedAmount = json["formattedAmount"].toString();
    data.full = json["full"].toString();
    data.forEmail = json["for_email"].toString();
}

void parseProduct(const QJsonValue &json, api::OrderProduct &data)
{
    data.status = json["status"].toString();
    parseProductPrice(json["price"], data.price);
    data.image = json["image"].toString();
    data.title = json["title"].toString();
    data.id = json["id"].toString();
    data.refunded = json["isRefunded"].toBool();
    parseOrderPrice(json["cashValue"], data.cashValue);
    parseOrderPrice(json["walletValue"], data.walletValue);
    data.preorder = json["isPreorder"].toBool();
    data.displayAutomaticRefundLink = json["displayAutomaticRefundLink"].toBool();
    if (!json["refundDate"].isNull())
    {
        data.refundDate = QDateTime::fromSecsSinceEpoch(json["refundDate"].toInt());
    }
}

void parseOrder(const QJsonValue &json, api::Order &data)
{
    data.publicId = json["publicId"].toString();
    data.date = QDateTime::fromSecsSinceEpoch(json["date"].toInt());
    data.moneybackGuarantee = json["moneybackGuarantee"].toBool();
    data.status = json["status"].toString();
    data.paymentMethod = json["paymentMethod"].toString();
    if (!json["validUntil"].isNull())
    {
        data.validUntil = QDateTime::fromSecsSinceEpoch(json["validUntil"].toInt());
    }
    data.checkoutLink = json["checkoutLink"].toString();
    data.receiptLink = json["receiptLink"].toString();
    parseOrderPrice(json["total"], data.total);
    parseOrderPrice(json["storeCreditUsed"], data.storeCreditUsed);
    if (json["giftRecipient"].isString())
    {
        data.giftRecipient = json["giftRecipient"].toString();
    }
    if (json["giftSender"].isString())
    {
        data.giftSender = json["giftSender"].toString();
    }
    auto products = json["products"].toArray();
    data.products.resize(products.count());
    for (std::size_t i = 0; i < products.count(); i++)
    {
        parseProduct(products[i], data.products[i]);
    }
    if (json["giftCode"].isString())
    {
        data.giftCode = json["giftCode"].toString();
    }
    data.resendable = json["isResendable"].toBool();
    data.statusPageUrl = json["statusPageUrl"].toString();
}

void parseGetOrdersHistoryResponse(const QJsonValue &json, api::GetOrdersHistoryResponse &data)
{
    auto orders = json["orders"].toArray();
    data.orders.resize(orders.count());
    for (std::size_t i = 0; i < orders.count(); i++)
    {
        parseOrder(orders[i], data.orders[i]);
    }
    data.totalPages = json["totalPages"].toInt();
}
