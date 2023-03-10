#ifndef STATISTICSSERIALIZATION_H
#define STATISTICSSERIALIZATION_H

#include <QJsonObject>

#include "../models/statistics.h"

void parseGetAchievementsResponse(const QJsonObject &json, api::GetAchievementsResponse &data);
void parseGetSessionsResponse(const QJsonObject &json, api::GetPlaySessionsResponse &data);

#endif // STATISTICSSERIALIZATION_H
