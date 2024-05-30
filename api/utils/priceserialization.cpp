#include "./priceserialization.h"

#include <QJsonArray>

void parseGetPricesResponse(const QJsonObject &json, api::GetPricesResponse &data)
{
    if (json["_embedded"]["items"].isArray())
    {

    }
    else if (json["_embedded"]["prices"].isArray())
    {
        auto prices = json["_embedded"]["prices"].toArray();
        for (const QJsonValue &item : std::as_const(prices))
        {
            auto currencyCode = item["currency"]["code"].toString();
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
}
