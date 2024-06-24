#ifndef STORESERIALIZATION_H
#define STORESERIALIZATION_H

#include <QJsonValue>

#include "../models/store.h"

void parseGetStoreCustomSectionResponse(const QJsonValue &json, api::GetStoreCustomSectionResponse &data,
                                        const QString &coverFormat);
void parseGetStoreDiscoverGamesResponse(const QJsonValue &json, api::GetStoreDiscoverGamesSectionResponse &data);
void parseGetStoreNowOnSaleResponse(const QJsonValue &json, api::GetStoreNowOnSaleResponse &data);
void parseGetStoreNowOnSaleSectionResponse(const QJsonValue &json, api::GetStoreNowOnSaleSectionResponse &data);
void parseGetStoreRecommendedDlcsResponse(const QJsonValue &json, api::GetStoreRecommendedDlcsResponse &data);

#endif // STORESERIALIZATION_H
