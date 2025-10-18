#ifndef STORESERIALIZATION_H
#define STORESERIALIZATION_H

#include <QJsonValue>

#include "../models/store.h"

void parseGetStoreAnnouncementSectionResponse(const QJsonValue &json, api::GetStoreAnnouncementSectionResponse &data);
void parseGetStoreDiscoverSectionResponse(const QJsonValue &json, api::GetStoreDiscoverGamesSectionResponse &data);
void parseGetStoreHighlightsSectionResponse(const QJsonValue &json, api::GetStoreHighlightsSectionResponse &data);
void parseGetStoreNewsSectionResponse(const QJsonValue &json, api::GetStoreNewsSectionResponse &data);
void parseGetStoreNowOnSaleResponse(const QJsonValue &json, api::GetStoreNowOnSaleResponse &data);
void parseGetStoreNowOnSaleSectionResponse(const QJsonValue &json, api::GetStoreNowOnSaleSectionResponse &data);
void parseGetStoreProductsSectionResponse(const QJsonValue &json, api::GetStoreProductsSectionResponse &data);
void parseGetStorePromoBannerSectionResponse(const QJsonValue &json, api::GetStorePromoBannerSectionResponse &data);
void parseGetStoreRankingSectionResponse(const QJsonValue &json, api::GetStoreRankingSectionResponse &data);
void parseGetStoreSectionsResponse(const QJsonValue &json, api::GetStoreSectionsResponse &data);

#endif // STORESERIALIZATION_H
