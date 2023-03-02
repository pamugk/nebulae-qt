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
        for (int i = 0; i < prices.count(); i++)
        {
            auto item = prices[i].toObject();
            auto currencyCode = item["currency"].toObject()["code"].toString();
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
