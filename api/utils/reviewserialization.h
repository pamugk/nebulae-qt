#ifndef REVIEWSERIALIZATION_H
#define REVIEWSERIALIZATION_H

#include <QJsonValue>

#include "../models/review.h"

void parseRatingResponse(const QJsonValue &json, api::GetRatingResponse &data);

void parseReviewsResponse(const QJsonValue &json, api::GetReviewsResponse &data);

#endif // REVIEWSERIALIZATION_H
