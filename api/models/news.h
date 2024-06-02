#ifndef NEWS_H
#define NEWS_H

#include <QDateTime>
#include <QString>
#include <QVector>

namespace api
{
    struct NewsItem
    {
        unsigned long long id;
        QString title;
        QString slug;
        QString link;
        QString body;
        QDateTime publishDate;
        QString language;
        unsigned int commentsCount;
        QString forumThreadLink;

        QString imageSmall;
        QString imageLarge;
    };

    struct GetNewsResponse
    {
        unsigned char totalCount;
        unsigned char limit;
        unsigned short pageToken;
        unsigned short nextPageToken;
        QVector<NewsItem> items;
    };
}

#endif // NEWS_H
