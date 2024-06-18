#ifndef PROTECTEDAUTHDATASTORAGE_H
#define PROTECTEDAUTHDATASTORAGE_H

#include "../api/authdatastorage.h"

class ProtectedAuthDataStorage : public api::AuthDataStorage
{
    Q_OBJECT

public:
    ProtectedAuthDataStorage(bool storeTokens);

    virtual void dropAuthData() override;
    virtual void getAuthData() override;
    virtual void setStoreAuthData(bool storeTokens) override;

public slots:
    virtual void setAuthData(const QVariantMap &data) override;

private:
    bool storeData;
};

#endif // PROTECTEDAUTHDATASTORAGE_H
