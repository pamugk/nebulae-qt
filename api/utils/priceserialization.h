#ifndef PRICE_SERIALIZATION_H
#define PRICE_SERIALIZATION_H

#include <QJsonValue>

#include "../models/price.h"

void parseGetPricesResponse(const QJsonValue &json, api::GetPricesResponse &data);

#endif // PRICE_SERIALIZATION_H
