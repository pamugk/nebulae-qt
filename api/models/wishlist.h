#ifndef WISHLIST_H
#define WISHLIST_H

#include "./product.h"

namespace api
{
    struct GetWishlistResponse
    {
        unsigned short page;
        unsigned int totalProducts;
        unsigned short totalPages;
        unsigned short productsPerPage;
        QVector<Product> products;
    };
}

#endif // WISHLIST_H
