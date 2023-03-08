#ifndef OWNEDPRODUCTSERIALIZATION_H
#define OWNEDPRODUCTSERIALIZATION_H

#include <QJsonObject>

#include "../models/ownedproduct.h"

void parseOwnedProductsResponse(const QJsonObject &json, api::GetOwnedProductsResponse &data);

#endif // OWNEDPRODUCTSERIALIZATION_H
