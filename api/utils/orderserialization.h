#ifndef ORDER_SERIALIZATION_H
#define ORDER_SERIALIZATION_H

#include <QJsonObject>

#include "../models/order.h"

void parseGetOrdersHistoryResponse(const QJsonObject &json, GetOrdersHistoryResponse &data);

#endif // ORDER_SERIALIZATION_H
