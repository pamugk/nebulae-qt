#ifndef STORESERIALIZATION_H
#define STORESERIALIZATION_H

#include <QJsonObject>

#include "../models/store.h"

void parseGetStoreCustomSectionResponse(const QJsonObject &json, api::GetStoreCustomSectionResponse &data,
                                        const QString &coverFormat);
void parseGetStoreDiscoverGamesResponse(const QJsonObject &json, api::GetStoreDiscoverGamesSectionResponse &data);
void parseGetStoreNowOnSaleResponse(const QJsonObject &json, api::GetStoreNowOnSaleResponse &data);

#endif // STORESERIALIZATION_H
