#ifndef REVIEWSERIALIZATION_H
#define REVIEWSERIALIZATION_H

#include <QJsonObject>

#include "../models/review.h"

void parseRatingResponse(const QJsonObject &json, api::GetRatingResponse &data);

void parseReviewsResponse(const QJsonObject &json, api::GetReviewsResponse &data);

#endif // REVIEWSERIALIZATION_H
