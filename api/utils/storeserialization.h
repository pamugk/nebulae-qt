#ifndef STORESERIALIZATION_H
#define STORESERIALIZATION_H

#include <QJsonObject>

#include "../models/store.h"

void parseGetStoreDiscoverGamesResponse(const QJsonObject &json, api::GetStoreDiscoverGamesSectionResponse &data);

#endif // STORESERIALIZATION_H
