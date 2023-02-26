#ifndef SERIALIZATION_H
#define SERIALIZATION_H

#include <QJsonObject>

#include "../models/wishlist.h"

void parseSearchWishlistResponse(const QJsonObject &json, api::GetWishlistResponse &data);

#endif // SERIALIZATION_H
