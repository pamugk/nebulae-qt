#ifndef TOKENSTORAGE_H
#define TOKENSTORAGE_H

#include <QString>
#include <QtPlugin>

namespace api
{
    class TokenStorage
    {
    public:
        virtual void setStoreTokens(bool storeTokens) = 0;
        virtual void setRefreshToken(const QString &refreshToken) = 0;
        virtual void setToken(const QString &token) = 0;
        const virtual std::tuple<QString, QString> tokens() const = 0;
    };
}

#endif // TOKENSTORAGE_H
