#ifndef RECOMMENDATIONSERIALIZATION_H
#define RECOMMENDATIONSERIALIZATION_H

#include <QJsonObject>

#include "../models/recommendation.h"

void parseRecommendationsResponse(const QJsonObject &json, api::GetRecommendationsResponse &data);

#endif // RECOMMENDATIONSERIALIZATION_H
