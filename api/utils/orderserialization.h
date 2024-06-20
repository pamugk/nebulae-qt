#ifndef ORDER_SERIALIZATION_H
#define ORDER_SERIALIZATION_H

#include <QJsonValue>

#include "../models/order.h"

void parseGetOrdersHistoryResponse(const QJsonValue &json, api::GetOrdersHistoryResponse &data);

#endif // ORDER_SERIALIZATION_H
