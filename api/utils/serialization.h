#ifndef SERIALIZATION_H
#define SERIALIZATION_H

#include <QJsonValue>

#include "../models/wishlist.h"

void parseSearchWishlistResponse(const QJsonValue &json, api::GetWishlistResponse &data);

#endif // SERIALIZATION_H
