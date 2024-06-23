#ifndef STATISTICSSERIALIZATION_H
#define STATISTICSSERIALIZATION_H

#include <QJsonValue>

#include "../models/statistics.h"

void parseGetAchievementsResponse(const QJsonValue &json, api::GetUserAchievementsResponse &data);
void parseGetUserGameTimeStatisticsResponse(const QJsonValue &json, api::GetUserGameTimeStatisticsResponse &data);

#endif // STATISTICSSERIALIZATION_H
