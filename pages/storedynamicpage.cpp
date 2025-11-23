#include "storedynamicpage.h"
#include "ui_storedynamicpage.h"

#include <QDesktopServices>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QNetworkReply>

#include "../api/utils/catalogserialization.h"
#include "../api/utils/storeserialization.h"
#include "../widgets/simpleproductitem.h"
#include "../widgets/storecatalogsection.h"
#include "../widgets/storeherobanner.h"
#include "../widgets/storesalecard.h"

StoreDynamicPage::StoreDynamicPage(QWidget *parent) :
    StoreBasePage(Page::STORE_DYNAMIC_PAGE, parent),
    promoId(),
    timerId(),
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
                        auto itemWidget = new SimpleProductItem(sectionScrollAreaContents);
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
                    connect(backgroundReply, &QNetworkReply::finished, backgroundReply, &QNetworkReply::deleteLater);
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

                if (data.endDate.isValid() && data.showCountdown)
                {
                    if (!timerId.has_value())
                    {
                        timerId = startTimer(std::chrono::seconds(1));
                    }
                    connect(this, &StoreDynamicPage::timeTicked, heroBannerWidget, [sectionWidget, heroBannerWidget, endDateTime = data.endDate](const QDateTime &currentDateTime)
                            {
                                if (endDateTime > currentDateTime)
                                {
                                    heroBannerWidget->setCountdownValue(std::chrono::duration_cast<std::chrono::seconds>(endDateTime - currentDateTime));
                                    sectionWidget->setVisible(true);
                                }
                                else
                                {
                                    sectionWidget->setVisible(false);
                                }
                            });
                }

                sectionWidget->layout()->addWidget(heroBannerWidget);
            }
            else if (type == QLatin1StringView("VERTICAL_BANNER_SECTION"))
            {
                auto resultJson = QJsonDocument::fromJson(QString(sectionReply->readAll()).toUtf8()).object();
                api::GetStoreVerticalBannerSectionResponse data;
                parseGetStoreVerticalBannerSectionResponse(resultJson, data);

                if (data.items.isEmpty())
                {
                    ui->resultScrollAreaContentsLayout->removeWidget(sectionWidget);
                    sectionWidget->deleteLater();
                }
                else
                {
                    sectionWidget->setLayout(new QVBoxLayout());
                    QScrollArea *nowOnSaleDealsScrollArea = new QScrollArea(sectionWidget);
                    nowOnSaleDealsScrollArea->setMinimumSize(548, 520);
                    nowOnSaleDealsScrollArea->setWidgetResizable(true);
                    QWidget *nowOnSaleDealsScrollAreaContents = new QWidget(nowOnSaleDealsScrollArea);
                    nowOnSaleDealsScrollAreaContents->setLayout(new QHBoxLayout());
                    nowOnSaleDealsScrollArea->setWidget(nowOnSaleDealsScrollAreaContents);
                    sectionWidget->layout()->addWidget(nowOnSaleDealsScrollArea);

                    if (!timerId.has_value())
                    {
                        timerId = startTimer(std::chrono::seconds(1));
                    }

                    auto systemLocale = QLocale::system();
                    for (const api::StoreVerticalBannerItem &item : std::as_const(data.items))
                    {
                        if (item.promoId != promoId)
                        {
                            auto dealCard = new StoreSaleCard(nowOnSaleDealsScrollAreaContents);
                            dealCard->setTitle(item.title);
                            dealCard->setDiscountUpTo(item.discountUpTo);
                            dealCard->setDiscount(item.discount);
                            dealCard->setColor(item.color);
                            if (!item.backgroundImage.isEmpty())
                            {
                                QString url = item.backgroundImage;
                                url.replace(QLatin1StringView(".jpg"), QLatin1StringView("_vertical_banner_256x486.webp"));
                                QNetworkReply *backgroundReply = apiClient->getAnything(url);
                                connect(dealCard, &QObject::destroyed, backgroundReply, &QNetworkReply::abort);
                                connect(backgroundReply, &QNetworkReply::finished, dealCard, [dealCard, backgroundReply]()
                                        {
                                            if (backgroundReply->error() == QNetworkReply::NoError)
                                            {
                                                QPixmap image;
                                                image.loadFromData(backgroundReply->readAll());
                                                dealCard->setBackgroundImage(image);
                                            }
                                            else if (backgroundReply->error() != QNetworkReply::OperationCanceledError)
                                            {
                                                qDebug() << backgroundReply->error()
                                                << backgroundReply->errorString()
                                                << QString(backgroundReply->readAll()).toUtf8();
                                            }
                                        });
                                connect(backgroundReply, &QNetworkReply::finished, backgroundReply, &QNetworkReply::deleteLater);
                            }
                            connect(dealCard, &StoreSaleCard::navigateToItem,
                                    this, [this, url = QUrl(item.url)]()
                                    {
                                        emit navigate({Page::STORE_DYNAMIC_PAGE, url.path()});
                                    });
                            connect(this, &StoreDynamicPage::timeTicked, dealCard, [dealCard, promoEndDateTime = item.promoEndDate](const QDateTime &currentDateTime)
                                    {
                                        if (promoEndDateTime > currentDateTime)
                                        {
                                            dealCard->setCountdownValue(std::chrono::duration_cast<std::chrono::seconds>(promoEndDateTime - currentDateTime));
                                            dealCard->setVisible(true);
                                        }
                                        else
                                        {
                                            dealCard->setVisible(false);
                                        }
                                    });
                            nowOnSaleDealsScrollAreaContents->layout()->addWidget(dealCard);
                        }
                    }

                    auto titleLabel = new QLabel(tr("Hurry up! There are even more sales!"), ui->resultScrollAreaContents);
                    titleLabel->setStyleSheet(QStringLiteral("font: 700 12pt; padding: 16px 0; border-bottom: 1px solid #bfbfbf;"));
                    ui->resultScrollAreaContentsLayout->insertWidget(ui->resultScrollAreaContentsLayout->indexOf(sectionWidget), titleLabel);
                }
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
    });
    connect(sectionReply, &QNetworkReply::finished, sectionReply, &QNetworkReply::deleteLater);
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

            promoId = data.config.promoId;
            if (data.config.endDate.isValid())
            {
                if (!timerId.has_value())
                {
                    timerId = startTimer(std::chrono::seconds(1));
                }
                connect(this, &StoreDynamicPage::timeTicked, this, [this, endDateTime = data.config.endDate](const QDateTime &currentDateTime)
                        {
                            if (endDateTime <= currentDateTime)
                            {
                                // TODO: hide page
                            }
                        });
            }

            for (const auto &section: std::as_const(data.sections))
            {
                if (section.sectionType == QLatin1StringView("PRODUCTS_SECTION")
                    || section.sectionType == QLatin1StringView("HERO_SECTION")
                    || section.sectionType == QLatin1StringView("VERTICAL_BANNER_SECTION"))
                {
                    getSection(section.id, section.sectionType);
                }
                else if (section.sectionType == QLatin1StringView("WISHLIST_SECTION"))
                {
                    int startIndex = ui->resultScrollAreaContentsLayout->count();
                    auto titleLabel = new QLabel(tr("From your wishlist"), ui->resultScrollAreaContents);
                    titleLabel->setStyleSheet(QStringLiteral("font: 700 12pt; padding: 16px 0; border-bottom: 1px solid #bfbfbf;"));
                    ui->resultScrollAreaContentsLayout->addWidget(titleLabel);
                    auto sectionScrollArea = new QScrollArea(ui->resultScrollAreaContents);
                    sectionScrollArea->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
                    sectionScrollArea->setMinimumHeight(273);
                    auto sectionScrollAreaContents = new QWidget(sectionScrollArea);
                    auto sectionScrollAreaContentsLayout = new QHBoxLayout(sectionScrollAreaContents);
                    sectionScrollAreaContentsLayout->setContentsMargins(6, 0, 6, 6);
                    sectionScrollAreaContentsLayout->setSpacing(24);
                    sectionScrollAreaContents->setLayout(sectionScrollAreaContentsLayout);
                    sectionScrollArea->setWidget(sectionScrollAreaContents);
                    ui->resultScrollAreaContentsLayout->addWidget(sectionScrollArea);
                    updateWishlistSection(startIndex, section.id);
                    connect(this, &StoreDynamicPage::authenticationStateChanged, sectionScrollArea, [this, startIndex, sectionId = section.id]()
                    {
                        updateWishlistSection(startIndex, sectionId);
                    });
                }
                else if (section.sectionType == QLatin1StringView("CATALOG_SECTION"))
                {
                    auto catalogSection = new StoreCatalogSection(ui->resultScrollAreaContents);
                    connect(catalogSection, &StoreCatalogSection::navigate, this, [this](NavigationDestination destination)
                    {
                        emit navigate(destination);
                    });
                    ui->resultScrollAreaContentsLayout->addWidget(catalogSection);

                    catalogSection->initialize(QMap<QString, QVariant>({ std::pair(QLatin1StringView("pageId"), pathHex), std::pair(QLatin1StringView("sectionId"), section.id) }), apiClient, true);
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
    });
    connect(sectionsReply, &QNetworkReply::finished, sectionsReply, &QNetworkReply::deleteLater);
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
        });
        connect(ownedProductsReply, &QNetworkReply::finished, ownedProductsReply, &QNetworkReply::deleteLater);

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
        });
        connect(wishlistReply, &QNetworkReply::finished, wishlistReply, &QNetworkReply::deleteLater);
    }
    else
    {
        ownedProducts.clear();
        emit ownedProductsChanged(ownedProducts);
        wishlist.clear();
        emit wishlistChanged(wishlist);
    }
}

void StoreDynamicPage::timerEvent(QTimerEvent *event)
{
    if (event->timerId() == timerId)
    {
        emit timeTicked(QDateTime::currentDateTime());
    }
}

void StoreDynamicPage::updateWishlistSection(int startIndex, const QString &sectionId)
{
    QWidget *titleLabel = ui->resultScrollAreaContentsLayout->itemAt(startIndex)->widget();
    QScrollArea *sectionScrollArea = static_cast<QScrollArea *>(ui->resultScrollAreaContentsLayout->itemAt(startIndex + 1)->widget());
    titleLabel->setVisible(false);
    sectionScrollArea->setVisible(false);
    while (!sectionScrollArea->widget()->layout()->isEmpty())
    {
        auto item = sectionScrollArea->widget()->layout()->itemAt(0);
        sectionScrollArea->widget()->layout()->removeItem(item);
        item->widget()->deleteLater();
        delete item;
    }

    if (apiClient->isAuthenticated())
    {
        api::CatalogFilter request({});
        request.onlyWishlisted = true;
        request.pageId = pathHex;
        request.sectionId = sectionId;
        const auto systemLocale = QLocale::system();
        QNetworkReply *wishlistedGamesReply = apiClient->searchCatalog(
            {}, request,
            QLocale::territoryToCode(systemLocale.territory()),
            systemLocale.name(QLocale::TagSeparator::Dash),
            systemLocale.currencySymbol(QLocale::CurrencyIsoCode), 1, 8);
        connect(this, &QObject::destroyed, wishlistedGamesReply, &QNetworkReply::abort);
        connect(this, &StoreDynamicPage::authenticationStateChanged, wishlistedGamesReply, &QNetworkReply::abort);
        connect(wishlistedGamesReply, &QNetworkReply::finished, this, [this, wishlistedGamesReply, titleLabel, sectionScrollArea]()
        {
            if (wishlistedGamesReply->error() == QNetworkReply::NoError)
            {
                auto resultJson = QJsonDocument::fromJson(QString(wishlistedGamesReply->readAll()).toUtf8()).object();
                api::SearchCatalogResponse data;
                parseSearchCatalogResponse(resultJson, data, QLatin1StringView("_product_tile_256.webp"));

                if (!data.products.isEmpty())
                {
                    for (const api::CatalogProduct &item : std::as_const(data.products))
                    {
                        auto itemWidget = new SimpleProductItem(sectionScrollArea->widget());
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
                        sectionScrollArea->widget()->layout()->addWidget(itemWidget);
                    }
                    sectionScrollArea->widget()->resize(sectionScrollArea->widget()->sizeHint());
                    titleLabel->setVisible(true);
                    sectionScrollArea->setVisible(true);
                }
            }
            else if (wishlistedGamesReply->error() != QNetworkReply::OperationCanceledError)
            {
                qDebug() << wishlistedGamesReply->error()
                    << wishlistedGamesReply->errorString()
                    << QString(wishlistedGamesReply->readAll()).toUtf8();
            }
        });
        connect(wishlistedGamesReply, &QNetworkReply::finished, wishlistedGamesReply, &QNetworkReply::deleteLater);
    }
}
