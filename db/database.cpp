 #include "database.h"

#include <QDebug>
#include <QDir>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QStandardPaths>

const auto INSERT_USER_RELEASE = QLatin1String(R"(
    INSERT INTO user_release(user_id, platform, release_id, created_at, owned_since, hidden, rating)
        VALUES(?, ?, ?, ?, ?, ?, ?)
        ON CONFLICT(user_id, platform, release_id) DO UPDATE
        SET owned_since = excluded.owned_since, hidden = excluded.hidden, rating = excluded.rating
)");

const auto INSERT_USER_RELEASE_TAG = QLatin1String(R"(
    INSERT INTO user_release_tag(user_id, platform, release_id, tag)
        VALUES(?, ?, ?, ?)
        ON CONFLICT DO NOTHING
)");

const auto USER_RELEASE_DDL = QLatin1String(R"(
CREATE TABLE IF NOT EXISTS user_release(
    user_id INT64 NOT NULL,
    platform TEXT NOT NULL,
    release_id INT64 NOT NULL,
    created_at NOT NULL,
    owned_since NULL,
    hidden BOOLEAN NULL,
    rating INTEGER NULL,
    CONSTRAINT 'UK_user_release_key' UNIQUE (user_id, platform, release_id));
)");

const auto USER_RELEASE_TAG_DDL = QLatin1String(R"(
CREATE TABLE IF NOT EXISTS user_release_tag(
    user_id INT64 NOT NULL,
    platform TEXT NOT NULL,
    release_id INT64 NOT NULL,
    tag TEXT NOT NULL,
    CONSTRAINT 'CK_user_release_tag'
        CHECK (trim([tag]) <> ''),
    CONSTRAINT 'FK_user_release_tag_user_release'
        FOREIGN KEY (user_id, platform, release_id)
        REFERENCES user_release (user_id, platform, release_id) ON DELETE CASCADE,
    CONSTRAINT 'UK_user_release_tag_key' UNIQUE (user_id, platform, release_id, tag));
)");

void db::initialize()
{
    QDir dataDirectory(QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation));
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(dataDirectory.filePath("library.db"));

    if (!db.open())
    {
        qDebug() << "Could not open database: " << db.lastError();
        exit(-1);
    }

    QSqlQuery initializationQuery;
    if (!initializationQuery.exec(USER_RELEASE_DDL))
    {
        qDebug() << "Could not create user release table: " << initializationQuery.lastError();
        exit(-1);
    }
    if (!initializationQuery.exec(USER_RELEASE_TAG_DDL))
    {
        qDebug() << "Could not create user release tags table: " << initializationQuery.lastError();
        exit(-1);
    }
}

void db::saveUserReleases(unsigned long long userId, const QVector<api::UserRelease> &releases)
{
    QSqlDatabase db = QSqlDatabase::database();
    QSqlQuery insertReleaseQuery;
    if (!insertReleaseQuery.prepare(INSERT_USER_RELEASE))
    {
        qDebug() << "Failed to prepare user releases persistence query";
    }
    QSqlQuery insertReleaseTagQuery;
    if (!insertReleaseTagQuery.prepare(INSERT_USER_RELEASE_TAG))
    {
        qDebug() << "Failed to prepare user releases tags persistence query";
    }
    for (const api::UserRelease &release : releases)
    {
        insertReleaseQuery.bindValue(0, userId);
        insertReleaseQuery.bindValue(1, release.platformId);
        insertReleaseQuery.bindValue(2, release.externalId);
        insertReleaseQuery.bindValue(3, release.dateCreated);
        insertReleaseQuery.bindValue(4, release.ownedSince);
        insertReleaseQuery.bindValue(5, release.hidden.has_value() ? release.hidden.value() : QVariant(QMetaType::fromType<bool>()));
        insertReleaseQuery.bindValue(6, release.rating.has_value() ? release.rating.value() : QVariant(QMetaType::fromType<int>()));
        if (!insertReleaseQuery.exec())
        {
            qDebug() << "Failed to persist release " << release.platformId << release.externalId << insertReleaseQuery.lastError();
        }

        for (const QString &tag : release.tags)
        {
            insertReleaseTagQuery.bindValue(0, 0);
            insertReleaseTagQuery.bindValue(1, release.platformId);
            insertReleaseTagQuery.bindValue(2, release.externalId);
            insertReleaseTagQuery.bindValue(3, tag);
            if (!insertReleaseTagQuery.exec())
            {
                qDebug() << "Failed to persist tag " << release.platformId << release.externalId << insertReleaseTagQuery.lastError();
            }
        }
    }
}
