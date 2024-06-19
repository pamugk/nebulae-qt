#ifndef DATABASE_H
#define DATABASE_H

#include <QVector>

#include "../api/models/userrelease.h"

namespace db
{
    void initialize();
    void saveUserReleases(unsigned long long userId, const QVector<api::UserRelease> &releases);
}

#endif
