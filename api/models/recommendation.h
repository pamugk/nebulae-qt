#ifndef RECOMMENDATION_H
#define RECOMMENDATION_H

#include <QVector>

namespace api
{
    struct Details
    {
        QString title;
        bool available;
        QString imageUrl;
        QString imageHorizontalUrl;
        QString storeUrl;
        double feedback;
    };

    struct Discount
    {
        QString id;
        unsigned int basisPoints;
    };

    struct Pricing
    {
        QString currency;
        QString country;
        bool priceSet;
        unsigned int basePrice;
        unsigned int finalPrice;
        Discount discount;
    };

    struct Recommendation
    {
        QString productId;
        double rating;
        Details details;
        Pricing pricing;
    };

    struct GetRecommendationsResponse
    {
        QString context;
        QString requestId;
        QVector<Recommendation> products;
    };
}

#endif // RECOMMENDATION_H
