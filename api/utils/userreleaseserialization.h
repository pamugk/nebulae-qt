#ifndef USERRELEASESERIALIZATION_H
#define USERRELEASESERIALIZATION_H

#include <QJsonValue>

#include "../models/userrelease.h"

void parseGetUserReleasesResponse(const QJsonValue &json, api::GetUserReleasesResponse &data);

#endif // USERRELEASESERIALIZATION_H
