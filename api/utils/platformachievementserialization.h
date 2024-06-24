#ifndef PLATFORMACHIEVEMENTSERIALIZATION_H
#define PLATFORMACHIEVEMENTSERIALIZATION_H

#include <QJsonValue>

#include "../models/platformachievement.h"

void parsePlatformAchievement(const QJsonValue &json, api::PlatformAchievement &data);
void parseGetPlatformReleaseAchievementsResponse(const QJsonValue &json, api::GetPlatformReleaseAchievementsResponse &data);

#endif // PLATFORMACHIEVEMENTSERIALIZATION_H
