#ifndef RECOMMENDATIONSERIALIZATION_H
#define RECOMMENDATIONSERIALIZATION_H

#include <QJsonValue>

#include "../models/recommendation.h"

void parseRecommendationsResponse(const QJsonValue &json, api::GetRecommendationsResponse &data);

#endif // RECOMMENDATIONSERIALIZATION_H
