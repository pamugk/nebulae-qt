#ifndef STATISTICSSERIALIZATION_H
#define STATISTICSSERIALIZATION_H

#include <QJsonValue>

#include "../models/statistics.h"

void parseGetUserPlatformAchievementsResponse(const QJsonValue &json, api::GetUserPlatformAchievementsResponse &data);
void parseGetUserGameTimeStatisticsResponse(const QJsonValue &json, api::GetUserGameTimeStatisticsResponse &data);

#endif // STATISTICSSERIALIZATION_H
