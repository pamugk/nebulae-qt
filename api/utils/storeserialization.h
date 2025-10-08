#ifndef STORESERIALIZATION_H
#define STORESERIALIZATION_H

#include <QJsonValue>

#include "../models/store.h"

void parseGetStoreProductsSectionResponse(const QJsonValue &json, api::GetStoreProductsSectionResponse &data);
void parseGetStoreSectionsResponse(const QJsonValue &json, api::GetStoreSectionsResponse &data);

#endif // STORESERIALIZATION_H
