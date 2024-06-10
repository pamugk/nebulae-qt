#include "protectedtokenstorage.h"

ProtectedTokenStorage::ProtectedTokenStorage(bool storeTokens) :
    storeTokens(storeTokens)
{
}

void ProtectedTokenStorage::setStoreTokens(bool storeTokens)
{
    this->storeTokens = storeTokens;
    // TODO: drop stored tokens
}

void ProtectedTokenStorage::setRefreshToken(const QString &refreshToken)
{
    // TODO: update refresh token
}

void ProtectedTokenStorage::setToken(const QString &token)
{
    // TODO: update token
}

const std::tuple<QString, QString> ProtectedTokenStorage::tokens() const
{
    return std::make_tuple(QString(), QString());
}
