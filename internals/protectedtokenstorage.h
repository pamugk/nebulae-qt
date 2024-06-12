#ifndef PROTECTEDTOKENSTORAGE_H
#define PROTECTEDTOKENSTORAGE_H

#include "../api/tokenstorage.h"

class ProtectedTokenStorage : public api::TokenStorage
{
    Q_OBJECT

public:
    ProtectedTokenStorage(bool storeTokens);
    virtual void setStoreTokens(bool storeTokens) override;
    void getTokens() override;

public slots:
    virtual void setRefreshToken(const QString &refreshToken) override;
    virtual void setToken(const QString &token) override;

private:
    bool storeTokens;
};

#endif // PROTECTEDTOKENSTORAGE_H
