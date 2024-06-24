#ifndef OWNEDPRODUCTINFOSERIALIZATION_H
#define OWNEDPRODUCTINFOSERIALIZATION_H

#include <QJsonValue>

#include "../models/ownedproductinfo.h"

void parseGetOwnedProductInfoResponse(const QJsonValue &json, api::GetOwnedProductInfoResponse &data);

#endif // OWNEDPRODUCTINFOSERIALIZATION_H
