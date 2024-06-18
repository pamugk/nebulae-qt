#ifndef USERRELEASESERIALIZATION_H
#define USERRELEASESERIALIZATION_H

#include <QJsonObject>

#include "../models/userrelease.h"

void parseGetUserReleasesResponse(const QJsonObject &json, api::GetUserReleasesResponse &data);

#endif // USERRELEASESERIALIZATION_H
