#ifndef REVIEWFILTERS_H
#define REVIEWFILTERS_H

#include <QDateTime>
#include <QSet>
#include <QString>
#include <QVector>

namespace api
{
    struct ReviewFilters
    {
        QVector<QString> allLanguages;
        QSet<QString> languages;
        bool otherLanguages;

        std::optional<unsigned char> lastDays;
        std::optional<bool> reviewedByOwner;
        std::optional<bool> reviewedDuringDevelopment;
    };
}

#endif // REVIEWFILTERS_H
