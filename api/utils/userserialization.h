#ifndef USERSERIALIZATION_H
#define USERSERIALIZATION_H

#include <QJsonObject>

#include "../models/user.h"

void parseUserData(const QJsonObject &json, api::User &data);
void parseUserFullData(const QJsonObject &json, api::UserFullData &data);

#endif // USERSERIALIZATION_H
