#ifndef STORESERIALIZATION_H
#define STORESERIALIZATION_H

#include <QJsonObject>

#include "../models/store.h"

void parseGetStoreCustomSectionResponse(const QJsonObject &json, api::GetStoreCustomSectionResponse &data);
void parseGetStoreDiscoverGamesResponse(const QJsonObject &json, api::GetStoreDiscoverGamesSectionResponse &data);

#endif // STORESERIALIZATION_H
