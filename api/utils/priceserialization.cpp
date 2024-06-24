#include "./priceserialization.h"

#include <QJsonArray>

void parseGetPricesResponse(const QJsonValue &json, api::GetPricesResponse &data)
{
    QJsonArray prices;
    if (json["_embedded"]["items"].isArray())
    {
        const QJsonValue &item = json["_embedded"]["items"].toArray()[0];
        prices = item["_embedded"]["prices"].toArray();
    }
    else
    {
        prices = json["_embedded"]["prices"].toArray();
    }

    data.currencies.reserve(prices.count());
    for (const QJsonValue &item : std::as_const(prices))
    {
        auto currencyCode = item["currency"]["code"].toString();
        data.currencies << currencyCode;
        data.prices[currencyCode] =
                api::ProductPrice
            {
                item["basePrice"].toString().remove(' ' + currencyCode).toUInt(),
                item["finalPrice"].toString().remove(' ' + currencyCode).toUInt(),
                item["bonusWalletFunds"].toString().remove(' ' + currencyCode).toUInt(),
                item["promoId"].toString()
            };
    }
}
