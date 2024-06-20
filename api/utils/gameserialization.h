#ifndef GAMESERIALIZATION_H
#define GAMESERIALIZATION_H

#include <QJsonValue>

#include "../models/game.h"

void parseGame(const QJsonValue &json, api::Game &data);
void parseIdMetaTag(const QJsonValue &json, api::IdMetaTag &data);
void parseLocalizedMetaTag(const QJsonValue &json, api::LocalizedMetaTag &data);
void parseNamedVideo(const QJsonValue &json, api::NamedVideo &data);
void parsePlatformRelease(const QJsonValue &json, api::PlatformRelease &data);

#endif // GAMESERIALIZATION_H
