#include "./priceserialization.h"

#include <QJsonArray>

void parseGetPricesResponse(const QJsonObject &json, api::GetPricesResponse &data)
{
    auto prices = json["_embedded"]["items"].isArray()
            ? json["_embedded"]["items"].toArray()[0].toObject()["_embedded"].toObject()["prices"].toArray()
            : json["_embedded"]["prices"].toArray();

    for (const QJsonValue &item : std::as_const(prices))
    {
        auto currencyCode = item["currency"]["code"].toString();
        data.currencies.append(currencyCode);
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
