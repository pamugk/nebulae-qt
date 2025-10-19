#include "storedynamicpage.h"
#include "ui_storedynamicpage.h"

#include <QDesktopServices>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QNetworkReply>

#include "../api/utils/storeserialization.h"
#include "../widgets/simpleproductitem.h"
#include "../widgets/storeherobanner.h"

StoreDynamicPage::StoreDynamicPage(QWidget *parent) :
    StoreBasePage(Page::STORE_DYNAMIC_PAGE, parent),
    ui(new Ui::StoreDynamicPage)
{
    ui->setupUi(this);
}

StoreDynamicPage::~StoreDynamicPage()
{
    delete ui;
}

void StoreDynamicPage::setApiClient(api::GogApiClient *apiClient)
{
    this->apiClient = apiClient;
}

void StoreDynamicPage::getSection(const QString &id, const QString &type)
{
    QWidget *sectionWidget = new QWidget(ui->resultScrollAreaContents);
    ui->resultScrollAreaContentsLayout->addWidget(sectionWidget);

    const auto systemLocale = QLocale::system();
    const auto sectionReply = apiClient->getStoreSection(pathHex, id,
                                                         systemLocale.name(QLocale::TagSeparator::Dash),
                                                         QLocale::territoryToCode(systemLocale.territory()),
                                                         systemLocale.currencySymbol(QLocale::CurrencyIsoCode));
    connect(this, &QObject::destroyed, sectionReply, &QNetworkReply::abort);
    connect(sectionReply, &QNetworkReply::finished, this, [this, sectionReply, sectionWidget, type]()
    {
        if (sectionReply->error() == QNetworkReply::NoError)
        {
            if (type == QLatin1StringView("PRODUCTS_SECTION"))
            {
                auto resultJson = QJsonDocument::fromJson(QString(sectionReply->readAll()).toUtf8()).object();
                api::GetStoreProductsSectionResponse data;
                parseGetStoreProductsSectionResponse(resultJson, data);

                if (data.items.isEmpty())
                {
                    ui->resultScrollAreaContentsLayout->removeWidget(sectionWidget);
                    sectionWidget->deleteLater();
                }
                else
                {
                    sectionWidget->setLayout(new QVBoxLayout());
                    auto sectionScrollArea = new QScrollArea(sectionWidget);
                    sectionScrollArea->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
                    sectionScrollArea->setMinimumHeight(273);
                    auto sectionScrollAreaContents = new QWidget(sectionScrollArea);
                    auto sectionScrollAreaContentsLayout = new QHBoxLayout(sectionScrollAreaContents);
                    sectionScrollAreaContentsLayout->setContentsMargins(6, 0, 6, 6);
                    sectionScrollAreaContentsLayout->setSpacing(24);
                    for (const api::CatalogProduct &item : std::as_const(data.items))
                    {
                        auto itemWidget = new SimpleProductItem(sectionScrollArea);
                        itemWidget->setCover(item.coverHorizontal, apiClient);
                        itemWidget->setTitle(item.title);
                        if (item.price.has_value())
                        {
                            itemWidget->setPrice(item.price->baseMoney.amount, item.price->finalMoney.amount,
                                                 100 - std::round((item.price->finalMoney.amount / item.price->baseMoney.amount) * 100),
                                                 item.price->finalMoney.amount == 0, "");
                        }
                        connect(this, &StoreDynamicPage::ownedProductsChanged,
                                itemWidget, [itemWidget, productId = item.id](const QSet<const QString> &ids)
                        {
                            itemWidget->setOwned(ids.contains(productId));
                        });
                        itemWidget->setOwned(ownedProducts.contains(item.id));
                        connect(this, &StoreDynamicPage::wishlistChanged,
                                itemWidget, [itemWidget, productId = item.id](const QSet<const QString> &ids)
                        {
                            itemWidget->setWishlisted(ids.contains(productId));
                        });
                        itemWidget->setWishlisted(wishlist.contains(item.id));
                        connect(apiClient, &api::GogApiClient::authenticated,
                                itemWidget, &SimpleProductItem::switchUiAuthenticatedState);
                        itemWidget->switchUiAuthenticatedState(apiClient->isAuthenticated());
                        connect(itemWidget, &SimpleProductItem::clicked,
                                this, [this, productId = item.id]()
                        {
                            emit navigate({Page::CATALOG_PRODUCT, productId});
                        });
                        sectionScrollAreaContentsLayout->addWidget(itemWidget);
                    }
                    sectionScrollAreaContentsLayout->addStretch();
                    sectionScrollArea->setWidget(sectionScrollAreaContents);
                    sectionWidget->layout()->addWidget(sectionScrollArea);

                    if (!data.title.isEmpty())
                    {
                        auto titleLabel = new QLabel(data.title, ui->resultScrollAreaContents);
                        titleLabel->setStyleSheet(QStringLiteral("font: 700 12pt; padding: 16px 0; border-bottom: 1px solid #bfbfbf;"));
                        ui->resultScrollAreaContentsLayout->insertWidget(ui->resultScrollAreaContentsLayout->indexOf(sectionWidget), titleLabel);
                    }
                }
            }
            else if (type == QLatin1StringView("HERO_SECTION"))
            {
                auto resultJson = QJsonDocument::fromJson(QString(sectionReply->readAll()).toUtf8()).object();
                api::GetStoreHeroSectionResponse data;
                parseGetStoreHeroSectionResponse(resultJson, data);
                sectionWidget->setLayout(new QHBoxLayout());

                auto heroBannerWidget = new StoreHeroBanner(sectionWidget);
                heroBannerWidget->setUseDarkTheme(data.theme == QLatin1StringView("dark"));
                heroBannerWidget->setTitle(data.title);
                if (!data.imageHash.isEmpty())
                {
                    QNetworkReply *backgroundReply = apiClient->getAnything(QLatin1StringView("https://images.gog-statics.com/%1_hero_2560x423.webp").arg(data.imageHash));
                    connect(heroBannerWidget, &QObject::destroyed, backgroundReply, &QNetworkReply::abort);
                    connect(backgroundReply, &QNetworkReply::finished, heroBannerWidget, [heroBannerWidget, backgroundReply]()
                    {
                        if (backgroundReply->error() == QNetworkReply::NoError)
                        {
                            heroBannerWidget->setBackgroundImage(backgroundReply->readAll());
                        }
                        else if (backgroundReply->error() != QNetworkReply::OperationCanceledError)
                        {
                            qDebug() << backgroundReply->error()
                                     << backgroundReply->errorString()
                                     << QString(backgroundReply->readAll()).toUtf8();
                        }
                    });
                }
                heroBannerWidget->setDescription(data.description);
                heroBannerWidget->setPrimaryButtonText(data.button.text);
                connect(heroBannerWidget, &StoreHeroBanner::primaryButtonClicked, this, [url = data.button.link, anchor = data.button.anchor]()
                {
                    if (!url.isEmpty())
                    {
                        QDesktopServices::openUrl(QUrl(url));
                    }
                    else if (!anchor.isEmpty())
                    {
                        // TODO: jump to anchor
                    }
                });

                sectionWidget->layout()->addWidget(heroBannerWidget);
            }
        }
        else if (sectionReply->error() != QNetworkReply::OperationCanceledError)
        {
            qDebug() << sectionReply->error()
                     << sectionReply->errorString()
                     << QString(sectionReply->readAll()).toUtf8();
            ui->resultScrollAreaContentsLayout->removeWidget(sectionWidget);
            sectionWidget->deleteLater();
        }

        sectionReply->deleteLater();
    });
}

void StoreDynamicPage::getSections()
{
    ui->pageStackedWidget->setCurrentWidget(ui->loadingPage);

    const auto systemLocale = QLocale::system();
    const auto sectionsReply = apiClient->getStoreSections(pathHex,
                                                           systemLocale.name(QLocale::TagSeparator::Dash),
                                                           QLocale::territoryToCode(systemLocale.territory()),
                                                           systemLocale.currencySymbol(QLocale::CurrencyIsoCode));
    connect(this, &QObject::destroyed, sectionsReply, &QNetworkReply::abort);
    connect(sectionsReply, &QNetworkReply::finished, this, [this, sectionsReply]()
    {
        if (sectionsReply->error() == QNetworkReply::NoError)
        {
            auto resultJson = QJsonDocument::fromJson(QString(sectionsReply->readAll()).toUtf8()).object();
            api::GetStoreSectionsResponse data;
            parseGetStoreSectionsResponse(resultJson, data);
            for (const auto &section: std::as_const(data.sections))
            {
                if (section.sectionType == QLatin1StringView("PRODUCTS_SECTION")
                    || section.sectionType == QLatin1StringView("HERO_SECTION")
                    || section.sectionType == QLatin1StringView("WISHLIST_SECTION")
                    || section.sectionType == QLatin1StringView("VERTICAL_BANNER_SECTION"))
                {
                    getSection(section.id, section.sectionType);
                }
                else if (section.sectionType == QLatin1StringView("CATALOG_SECTION"))
                {

                }
            }
            ui->resultScrollAreaContentsLayout->addStretch();

            ui->pageStackedWidget->setCurrentWidget(ui->resultPage);
        }
        else if (sectionsReply->error() != QNetworkReply::OperationCanceledError)
        {
            qDebug() << sectionsReply->error()
                     << sectionsReply->errorString()
                     << QString(sectionsReply->readAll()).toUtf8();
            ui->pageStackedWidget->setCurrentWidget(ui->errorPage);
        }

        sectionsReply->deleteLater();
    });
}

void StoreDynamicPage::initialize(const QVariant &data)
{
    pathHex = QString(data.toString().toLatin1().toHex());
    getSections();
}

void StoreDynamicPage::switchUiAuthenticatedState(bool authenticated)
{
    emit authenticationStateChanged();
    StoreBasePage::switchUiAuthenticatedState(authenticated);

    if (authenticated)
    {
        const auto ownedProductsReply = apiClient->getOwnedLicensesIds();
        connect(this, &QObject::destroyed, ownedProductsReply, &QNetworkReply::abort);
        connect(this, &StoreDynamicPage::authenticationStateChanged, ownedProductsReply, &QNetworkReply::abort);
        connect(ownedProductsReply, &QNetworkReply::finished, this, [this, ownedProductsReply]()
        {
            if (ownedProductsReply->error() == QNetworkReply::NoError)
            {
                auto resultJson = QJsonDocument::fromJson(QString(ownedProductsReply->readAll()).toUtf8());
                auto ownedProducts = resultJson.toVariant().toList();
                for (const QVariant &id : std::as_const(ownedProducts))
                {
                    this->ownedProducts.insert(id.toString());
                }
                emit ownedProductsChanged(this->ownedProducts);
            }
            else if (ownedProductsReply->error() != QNetworkReply::OperationCanceledError)
            {
                qDebug() << ownedProductsReply->error()
                         << ownedProductsReply->errorString()
                         << QString(ownedProductsReply->readAll()).toUtf8();
            }

            ownedProductsReply->deleteLater();
        });

        const auto wishlistReply = apiClient->getWishlistIds();
        connect(this, &QObject::destroyed, wishlistReply, &QNetworkReply::abort);
        connect(this, &StoreDynamicPage::authenticationStateChanged, wishlistReply, &QNetworkReply::abort);
        connect(wishlistReply, &QNetworkReply::finished, this, [this, wishlistReply]()
        {
            if (wishlistReply->error() == QNetworkReply::NoError)
            {
                auto resultJson = QJsonDocument::fromJson(QString(wishlistReply->readAll()).toUtf8());
                auto wishlistedItems = resultJson["wishlist"].toObject();
                for (const QString &key : wishlistedItems.keys())
                {
                    if (wishlistedItems[key].toBool())
                    {
                        wishlist.insert(key);
                    }
                }
                emit wishlistChanged(wishlist);
            }
            else if (wishlistReply->error() != QNetworkReply::OperationCanceledError)
            {
                qDebug() << wishlistReply->error()
                         << wishlistReply->errorString()
                         << QString(wishlistReply->readAll()).toUtf8();
            }

            wishlistReply->deleteLater();
        });
    }
    else
    {
        ownedProducts.clear();
        emit ownedProductsChanged(ownedProducts);
        wishlist.clear();
        emit wishlistChanged(wishlist);
    }
}
