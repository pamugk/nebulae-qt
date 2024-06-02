#ifndef REVIEW_H
#define REVIEW_H

#include <QDateTime>
#include <QVector>

namespace api
{
    struct Reviewer
    {
        QString id;
        QString username;
        QMap<QString, QString> avatar;
        struct {
            unsigned int games;
            unsigned int reviews;
        } counters;
    };

    struct Review
    {
        QString id;
        QString productId;
        qreal rating;
        QString contentTitle;
        QString contentDescription;
        QString contentLanguage;
        QVector<QString> labels;
        unsigned int downvotes;
        unsigned int upvotes;
        QDateTime date;
        QDateTime creationDate;
        Reviewer reviewer;
    };

    struct GetReviewsResponse
    {
        bool reviewable;
        unsigned short page;
        unsigned short limit;
        unsigned short pages;
        unsigned int reviewCount;
        double overallAvgRating;
        double filteredAvgRating;
        QString mostHelpfulReviewId;
        QVector<Review> items;
    };

    struct GetRatingResponse
    {
        double value;
        unsigned int count;
    };
}

#endif // REVIEW_H
