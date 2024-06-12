#ifndef TOKENSTORAGE_H
#define TOKENSTORAGE_H

#include <QString>
#include <QObject>

namespace api
{
    class TokenStorage : public QObject
    {
        Q_OBJECT

    public:
        virtual void setStoreTokens(bool storeTokens) = 0;
        virtual void getTokens() = 0;

    public slots:
        virtual void setRefreshToken(const QString &refreshToken) = 0;
        virtual void setToken(const QString &token) = 0;

    signals:
        void refreshTokenAcquired(const QString &token);
        void tokenAcquired(const QString &token);
        void tokensRequested();
    };
}

#endif // TOKENSTORAGE_H
