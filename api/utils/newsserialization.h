#ifndef NEWSSERIALIZATION_H
#define NEWSSERIALIZATION_H

#include <QJsonObject>

#include "../models/news.h"

void parseNewsResponse(const QJsonObject &json, api::GetNewsResponse &data);

#endif // NEWSSERIALIZATION_H
