#ifndef AUTHDATASTORAGE_H
#define AUTHDATASTORAGE_H

#include <QObject>
#include <QVariantMap>

namespace api
{
    class AuthDataStorage : public QObject
    {
        Q_OBJECT

    public:
        virtual void getAuthData() = 0;
        virtual void setStoreAuthData(bool storeTokens) = 0;

    public slots:
        virtual void setAuthData(const QVariantMap &data) = 0;

    signals:
        void authDataAcquired(const QVariantMap &data);
        void authDataRequested();
    };
}

#endif // AUTHDATASTORAGE_H
