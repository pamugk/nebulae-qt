#ifndef REVIEW_H
#define REVIEW_H

#include <QtGlobal>

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
            quint32 games;
            quint32 reviews;
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
        quint32 downvotes;
        quint32 upvotes;
        QDateTime date;
        QDateTime creationDate;
        Reviewer reviewer;
    };

    struct GetReviewsResponse
    {
        bool reviewable;
        quint16 page;
        quint16 limit;
        quint16 pages;
        quint32 reviewCount;
        qreal overallAvgRating;
        qreal filteredAvgRating;
        QString mostHelpfulReviewId;
        QVector<Review> items;
    };

    struct GetRatingResponse
    {
        qreal value;
        quint32 count;
    };
}

#endif // REVIEW_H
