#include "protectedtokenstorage.h"
#include <QEventLoop>
#include <qt6keychain/keychain.h>

ProtectedTokenStorage::ProtectedTokenStorage(bool storeTokens) :
    storeTokens(storeTokens)
{
}

void ProtectedTokenStorage::setStoreTokens(bool storeTokens)
{
    bool wasStoringTokens = this->storeTokens;
    this->storeTokens = storeTokens;
    if (wasStoringTokens && !storeTokens)
    {
        auto deleteTokenJob = new QKeychain::DeletePasswordJob("com.github.pamugk.Nebulae");
        deleteTokenJob->setKey("token");
        deleteTokenJob->start();
        auto deleteRefreshTokenJob = new QKeychain::DeletePasswordJob("com.github.pamugk.Nebulae");
        deleteRefreshTokenJob->setKey("refreshToken");
        deleteRefreshTokenJob->start();
    }
    else if (!wasStoringTokens && storeTokens)
    {
        emit tokensRequested();
    }
}

void ProtectedTokenStorage::setRefreshToken(const QString &refreshToken)
{
    if (storeTokens)
    {
        auto job = new QKeychain::WritePasswordJob("com.github.pamugk.Nebulae");
        job->setKey("refreshToken");
        job->setTextData(refreshToken);
        job->start();
    }
}

void ProtectedTokenStorage::setToken(const QString &token)
{
    if (storeTokens)
    {
        auto job = new QKeychain::WritePasswordJob("com.github.pamugk.Nebulae");
        job->setKey("token");
        job->setTextData(token);
        job->start();
    }
}

void ProtectedTokenStorage::getTokens()
{
    auto refreshTokenJob = new QKeychain::ReadPasswordJob("com.github.pamugk.Nebulae");
    refreshTokenJob->setKey("refreshToken");
    QObject::connect(refreshTokenJob, &QKeychain::ReadPasswordJob::finished, this, [this, refreshTokenJob]()
    {
        if (refreshTokenJob->error() == QKeychain::Error::NoError)
        {
            emit refreshTokenAcquired(refreshTokenJob->textData());
        }
        else
        {
            emit refreshTokenAcquired(QString());
        }
    });
    refreshTokenJob->start();

    auto tokenJob = new QKeychain::ReadPasswordJob("com.github.pamugk.Nebulae");
    tokenJob->setKey("token");
    QObject::connect(tokenJob, &QKeychain::ReadPasswordJob::finished, this, [this, tokenJob]()
    {
        if (tokenJob->error() == QKeychain::Error::NoError)
        {
            emit tokenAcquired(tokenJob->textData());
        }
        else
        {
            emit tokenAcquired(QString());
        }
    });
    tokenJob->start();
}
