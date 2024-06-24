#ifndef SERIESGAMESERIALIZATION_H
#define SERIESGAMESERIALIZATION_H

#include <QJsonValue>

#include "../models/seriesgame.h"

void parseGetSeriesGamesResponse(const QJsonValue &json, api::GetSeriesGamesResponse &data);

#endif // SERIESGAMESERIALIZATION_H
