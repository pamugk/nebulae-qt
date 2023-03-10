#ifndef OWNEDPRODUCTINFOSERIALIZATION_H
#define OWNEDPRODUCTINFOSERIALIZATION_H

#include <QJsonObject>

#include "../models/ownedproductinfo.h"

void parseGetOwnedProductInfoResponse(const QJsonObject &json, api::GetOwnedProductInfoResponse &data);

#endif // OWNEDPRODUCTINFOSERIALIZATION_H
