#ifndef NEWSSERIALIZATION_H
#define NEWSSERIALIZATION_H

#include <QJsonValue>

#include "../models/news.h"

void parseNewsResponse(const QJsonValue &json, api::GetNewsResponse &data);

#endif // NEWSSERIALIZATION_H
