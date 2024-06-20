#ifndef RELEASESERIALIZATION_H
#define RELEASESERIALIZATION_H

#include <QJsonValue>

#include "../models/release.h"

void parseRelease(const QJsonValue &json, api::Release &data);

#endif // RELEASESERIALIZATION_H
