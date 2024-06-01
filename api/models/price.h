#ifndef PRICE_H
#define PRICE_H

#include <QMap>
#include <QString>

namespace api
{
    struct ProductPrice
    {
        unsigned int basePrice;
        unsigned int finalPrice;
        unsigned int bonusWalletFunds;
        QString promoId;
    };

    struct GetPricesResponse
    {
        QVector<QString> currencies;
        QMap<QString, ProductPrice> prices;
    };
}

#endif // PRICE_H
