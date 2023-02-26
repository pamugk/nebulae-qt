#ifndef WISHLIST_H
#define WISHLIST_H

#include "./product.h"

namespace api
{
    struct GetWishlistResponse
    {
        quint16 page;
        quint32 totalProducts;
        quint16 totalPages;
        quint16 productsPerPage;
        QVector<Product> products;
    };
}

#endif // WISHLIST_H
