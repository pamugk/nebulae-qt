#include "protectedauthdatastorage.h"

#include <QBuffer>
#include <QByteArray>
#include <QDataStream>
#include <qt6keychain/keychain.h>

ProtectedAuthDataStorage::ProtectedAuthDataStorage(bool storeTokens) :
    storeData(storeTokens)
{
}

void ProtectedAuthDataStorage::dropAuthData()
{
    auto deleteTokenJob = new QKeychain::DeletePasswordJob("com.github.pamugk.Nebulae");
    deleteTokenJob->start();
}

void ProtectedAuthDataStorage::getAuthData()
{
    auto job = new QKeychain::ReadPasswordJob("com.github.pamugk.Nebulae");
    QObject::connect(job, &QKeychain::ReadPasswordJob::finished, this, [this, job]()
    {
        if (job->error() == QKeychain::Error::NoError)
        {
            QVariantMap data;
            QByteArray serializedData = job->binaryData();
            QBuffer dataBuffer(&serializedData);
            dataBuffer.open(QIODevice::ReadOnly);
            QDataStream dataDeserializer(&dataBuffer);
            dataDeserializer >> data;
            emit authDataAcquired(data);
        }
        else
        {
            emit authDataAcquired(QVariantMap());
        }
    });
    job->start();
}

void ProtectedAuthDataStorage::setStoreAuthData(bool storeTokens)
{
    bool wasStoringData = this->storeData;
    this->storeData = storeTokens;
    if (wasStoringData && !storeTokens)
    {
        auto deleteTokenJob = new QKeychain::DeletePasswordJob("com.github.pamugk.Nebulae");
        deleteTokenJob->start();
    }
    else if (!wasStoringData && storeTokens)
    {
        emit authDataRequested();
    }
}

void ProtectedAuthDataStorage::setAuthData(const QVariantMap &data)
{
    if (storeData)
    {
        QByteArray serializedData;
        QBuffer dataBuffer(&serializedData);
        dataBuffer.open(QIODevice::WriteOnly);
        QDataStream dataSerializer(&dataBuffer);
        dataSerializer << data;
        dataBuffer.close();

        auto job = new QKeychain::WritePasswordJob("com.github.pamugk.Nebulae");
        job->setBinaryData(serializedData);
        job->start();
    }
}
