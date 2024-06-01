#ifndef SERIESGAMESERIALIZATION_H
#define SERIESGAMESERIALIZATION_H

#include <QJsonObject>

#include "../models/seriesgame.h"

void parseGetSeriesGamesResponse(const QJsonObject &json, api::GetSeriesGamesResponse &data);

#endif // SERIESGAMESERIALIZATION_H
