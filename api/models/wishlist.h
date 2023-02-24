#ifndef WISHLIST_H
#define WISHLIST_H

#include "./product.h"

enum class WishlistSortOrder
{
    DATE_ADDED,
    TITLE,
    USER_REVIEWS
};

struct GetWishlistResponse
{
    quint16 page;
    quint32 totalProducts;
    quint16 totalPages;
    quint16 productsPerPage;
    QVector<Product> products;
};

#endif // WISHLIST_H
