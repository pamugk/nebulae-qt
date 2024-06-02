#ifndef ORDER_H
#define ORDER_H

#include <QDateTime>
#include <QVector>

namespace api
{
    struct OrderPrice
    {
        QString amount;
        QString symbol;
        QString code;
        bool zero;
        unsigned int rawAmount;
        QString formattedAmount;
        QString full;
        QString forEmail;
    };

    struct OrderProductPrice
    {
        QString baseAmount;
        QString amount;
        bool free;
        bool discounted;
        QString symbol;
    };

    struct OrderProduct
    {
        QString status;
        OrderProductPrice price;
        QString image;
        QString title;
        QString id;
        bool refunded;
        OrderPrice cashValue;
        OrderPrice walletValue;
        bool preorder;
        bool displayAutomaticRefundLink;
        QDateTime refundDate;
    };

    struct Order
    {
        QString publicId;
        QDateTime date;
        bool moneybackGuarantee;
        QString status;
        QString paymentMethod;
        QDateTime validUntil;
        QString checkoutLink;
        QString receiptLink;
        OrderPrice total;
        OrderPrice storeCreditUsed;
        QString giftRecipient;
        QString giftSender;
        QVector<OrderProduct> products;
        QString giftCode;
        bool resendable;
        QString statusPageUrl;
    };

    struct GetOrdersHistoryResponse
    {
        QVector<Order> orders;
        unsigned short totalPages;
    };
}

#endif // ORDER_H
