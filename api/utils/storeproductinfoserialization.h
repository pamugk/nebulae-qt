#ifndef STOREPRODUCTINFOSERIALIZATION_H
#define STOREPRODUCTINFOSERIALIZATION_H

#include <QJsonValue>

#include "../models/storeproductinfo.h"

void parseStoreOwnedProductInfoResponse(const QJsonValue &json, api::GetStoreProductInfoResponse &data);

#endif // STOREPRODUCTINFOSERIALIZATION_H
