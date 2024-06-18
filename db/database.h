#ifndef DATABASE_H
#define DATABASE_H

#include <QVector>

#include "../api/models/userrelease.h"

namespace db
{
    void initialize();
    void saveUserReleases(const QVector<api::UserRelease> &releases);
}

#endif