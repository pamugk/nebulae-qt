#ifndef STATISTICSSERIALIZATION_H
#define STATISTICSSERIALIZATION_H

#include <QJsonValue>

#include "../models/statistics.h"

void parseGetAchievementsResponse(const QJsonValue &json, api::GetUserAchievementsResponse &data);
void parseGetSessionsResponse(const QJsonValue &json, api::GetPlaySessionsResponse &data);

#endif // STATISTICSSERIALIZATION_H
