#ifndef USERSERIALIZATION_H
#define USERSERIALIZATION_H

#include <QJsonValue>

#include "../models/user.h"

void parseUserData(const QJsonValue &json, api::User &data);
void parseUserFullData(const QJsonValue &json, api::UserFullData &data);

#endif // USERSERIALIZATION_H
