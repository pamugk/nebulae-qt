#ifndef PROTECTEDTOKENSTORAGE_H
#define PROTECTEDTOKENSTORAGE_H

#include "../api/tokenstorage.h"

class ProtectedTokenStorage : public api::TokenStorage
{
public:
    ProtectedTokenStorage(bool storeTokens);

    virtual void setStoreTokens(bool storeTokens) override;
    virtual void setRefreshToken(const QString &refreshToken) override;
    virtual void setToken(const QString &token) override;
    const virtual std::tuple<QString, QString> tokens() const override;

private:
    bool storeTokens;
};

#endif // PROTECTEDTOKENSTORAGE_H
