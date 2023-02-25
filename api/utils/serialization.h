#ifndef SERIALIZATION_H
#define SERIALIZATION_H

#include <QJsonObject>

#include "../models/wishlist.h"

void parseSearchWishlistResponse(const QJsonObject &json, GetWishlistResponse &data);

#endif // SERIALIZATION_H
