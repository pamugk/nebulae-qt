#include "allgamespage.h"
#include "ui_allgamespage.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkReply>
#include <QVBoxLayout>

#include "../widgets/storecatalogsection.h"

AllGamesPage::AllGamesPage(const NavigationDestination &destination, QWidget *parent) :
    StoreBasePage(destination, parent),
    ui(new Ui::AllGamesPage)
{
    ui->setupUi(this);
    setLayout(new QVBoxLayout());
    layout()->setContentsMargins(0, 0, 0, 0);

    auto contentsWidget = new QWidget(this);
    contentsWidget->setLayout(new QVBoxLayout());
    contentsWidget->layout()->setContentsMargins(0, 0, 0, 0);
    layout()->addWidget(contentsWidget);
}

AllGamesPage::~AllGamesPage()
{
    delete ui;
}

void AllGamesPage::setApiClient(api::GogApiClient *apiClient)
{
    this->apiClient = apiClient;
}

void AllGamesPage::initialize(const QVariant &data)
{
    QWidget *contentsWidget = layout()->itemAt(0)->widget();
    auto catalogSection = new StoreCatalogSection(contentsWidget);
    connect(catalogSection, &StoreCatalogSection::navigate, this, [this](NavigationDestination destination)
    {
        emit navigate(destination);
    });
    connect(this, &AllGamesPage::ownedProductsChanged, catalogSection, &StoreCatalogSection::updateOwnedProducts);
    connect(this, &AllGamesPage::wishlistChanged, catalogSection, &StoreCatalogSection::updateWishlist);
    contentsWidget->layout()->addWidget(catalogSection);

    catalogSection->initialize(data, apiClient, false);
}

void AllGamesPage::switchUiAuthenticatedState(bool authenticated)
{
    StoreBasePage::switchUiAuthenticatedState(authenticated);
    emit authenticationStateChanged();
    if (authenticated)
    {
        QNetworkReply *ownedProductsReply = apiClient->getOwnedLicensesIds();
        connect(this, &QObject::destroyed, ownedProductsReply, &QNetworkReply::abort);
        connect(this, &AllGamesPage::authenticationStateChanged, ownedProductsReply, &QNetworkReply::abort);
        connect(ownedProductsReply, &QNetworkReply::finished, this, [this, ownedProductsReply]()
        {
            if (ownedProductsReply->error() == QNetworkReply::NoError)
            {
                auto resultJson = QJsonDocument::fromJson(QString(ownedProductsReply->readAll()).toUtf8());
                auto ownedProducts = resultJson.toVariant().toList();

                auto ids = QSet<const QString>();
                for (const QVariant &id : std::as_const(ownedProducts))
                {
                    ids.insert(id.toString());
                }
                emit ownedProductsChanged(ids);
            }
            else if (ownedProductsReply->error() != QNetworkReply::OperationCanceledError)
            {
                qDebug() << ownedProductsReply->error()
                         << ownedProductsReply->errorString()
                         << QString(ownedProductsReply->readAll()).toUtf8();
            }
        });
        connect(ownedProductsReply, &QNetworkReply::finished, ownedProductsReply, &QNetworkReply::deleteLater);

        QNetworkReply *wishlistReply = apiClient->getWishlistIds();
        connect(this, &QObject::destroyed, wishlistReply, &QNetworkReply::abort);
        connect(this, &AllGamesPage::authenticationStateChanged, wishlistReply, &QNetworkReply::abort);
        connect(wishlistReply, &QNetworkReply::finished, this, [this, wishlistReply]()
        {
            if (wishlistReply->error() == QNetworkReply::NoError)
            {
                auto resultJson = QJsonDocument::fromJson(QString(wishlistReply->readAll()).toUtf8());
                auto wishlistedItems = resultJson["wishlist"].toObject();

                auto ids = QSet<const QString>();
                for (const QString &key : wishlistedItems.keys())
                {
                    if (wishlistedItems[key].toBool())
                    {
                        ids.insert(key);
                    }
                }
                emit wishlistChanged(ids);
            }
            else if (wishlistReply->error() != QNetworkReply::OperationCanceledError)
            {
                qDebug() << wishlistReply->error()
                         << wishlistReply->errorString()
                         << QString(wishlistReply->readAll()).toUtf8();
            }
        });
        connect(wishlistReply, &QNetworkReply::finished, wishlistReply, &QNetworkReply::deleteLater);
    }
    else
    {
        emit ownedProductsChanged(QSet<const QString>());
        emit wishlistChanged(QSet<const QString>());
    }
}

