#ifndef OWNEDPRODUCTSERIALIZATION_H
#define OWNEDPRODUCTSERIALIZATION_H

#include <QJsonValue>

#include "../models/ownedproduct.h"

void parseOwnedProductsResponse(const QJsonValue &json, api::GetOwnedProductsResponse &data);

#endif // OWNEDPRODUCTSERIALIZATION_H
