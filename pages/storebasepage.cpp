#include "storebasepage.h"

#include <QHBoxLayout>
#include <QPushButton>

StoreBasePage::StoreBasePage(Page page, QWidget *parent)
    : BasePage(parent)
{
    auto navigationActionSet = new QWidget(this);
    auto navigationActionSetLayout = new QHBoxLayout;
    navigationActionSetLayout->setContentsMargins(0, 0, 0, 0);
    navigationActionSetLayout->setSpacing(0);
    navigationActionSet->setLayout(navigationActionSetLayout);
    auto storeNavigationButton = new QPushButton("Store", navigationActionSet);
    storeNavigationButton->setFlat(true);
    connect(storeNavigationButton, &QPushButton::clicked, this, [this]()
    {
        emit navigate({ Page::STORE });
    });
    navigationActionSet->layout()->addWidget(storeNavigationButton);
    auto allGamesNavigationButton = new QPushButton("All games", navigationActionSet);
    allGamesNavigationButton->setFlat(true);
    connect(allGamesNavigationButton, &QPushButton::clicked, this, [this]()
    {
        emit navigate({ Page::ALL_GAMES });
    });
    navigationActionSet->layout()->addWidget(allGamesNavigationButton);
    auto wishlistNavigationButton = new QPushButton("Wishlist", navigationActionSet);
    wishlistNavigationButton->setFlat(true);
    connect(wishlistNavigationButton, &QPushButton::clicked, this, [this]()
    {
        emit navigate({ Page::WISHLIST });
    });
    navigationActionSet->layout()->addWidget(wishlistNavigationButton);
    auto ordersNavigationButton = new QPushButton("Orders", navigationActionSet);
    ordersNavigationButton->setFlat(true);
    connect(ordersNavigationButton, &QPushButton::clicked, this, [this]()
    {
        emit navigate({ Page::ORDER_HISTORY });
    });
    navigationActionSet->layout()->addWidget(ordersNavigationButton);
    actions.append(navigationActionSet);
    switch (page)
    {
    case Page::STORE:
        storeNavigationButton->setStyleSheet("color: rgb(172, 59, 176);");
        break;
    case Page::ALL_GAMES:
        allGamesNavigationButton->setStyleSheet("color: rgb(172, 59, 176);");
        break;
    case Page::WISHLIST:
        wishlistNavigationButton->setStyleSheet("color: rgb(172, 59, 176);");
        break;
    case Page::ORDER_HISTORY:
        ordersNavigationButton->setStyleSheet("color: rgb(172, 59, 176);");
        break;
    default:
        break;
    }

    auto cartButton = new QPushButton(this);
    cartButton->setFlat(true);
    cartButton->setIcon(QIcon(":/icons/cart-shopping.svg"));
    connect(cartButton, &QPushButton::clicked, this, [this]()
    {
        emit navigate({ Page::CART });
    });
    actions.append(cartButton);
}

StoreBasePage::~StoreBasePage()
{
    actions.clear();
}

const QVector<QWidget *> StoreBasePage::getHeaderControls()
{
    return actions;
}

void StoreBasePage::switchUiAuthenticatedState(bool authenticated)
{
    if (!actions.isEmpty())
    {
        auto navigationSet = actions[0];
        navigationSet->layout()->itemAt(2)->widget()->setVisible(authenticated);
        navigationSet->layout()->itemAt(3)->widget()->setVisible(authenticated);
        actions[1]->setVisible(authenticated);
    }
}
