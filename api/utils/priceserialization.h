#ifndef PRICE_SERIALIZATION_H
#define PRICE_SERIALIZATION_H

#include <QJsonObject>

#include "../models/price.h"

void parseGetPricesResponse(const QJsonObject &json, api::GetPricesResponse &data);

#endif // PRICE_SERIALIZATION_H
