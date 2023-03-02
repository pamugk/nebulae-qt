#ifndef PRICE_H
#define PRICE_H

#include <QMap>
#include <QString>

namespace api
{
    struct ProductPrice
    {
        quint32 basePrice;
        quint32 finalPrice;
        quint32 bonusWalletFunds;
        QString promoId;
    };

    struct GetPricesResponse
    {
        QMap<QString, ProductPrice> prices;
    };
}

#endif // PRICE_H
