#ifndef RECOMMENDATIONSERIALIZATION_H
#define RECOMMENDATIONSERIALIZATION_H

#include <QJsonObject>

#include "../models/recommendation.h"

void parseCatalogProductInfoResponse(const QJsonObject &json, api::GetRecommendationsResponse &data);

#endif // RECOMMENDATIONSERIALIZATION_H
