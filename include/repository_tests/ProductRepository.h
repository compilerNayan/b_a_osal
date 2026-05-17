#ifndef _PRODUCT_REPOSITORY_H_
#define _PRODUCT_REPOSITORY_H_

#include <StandardDefines.h>
#include "CpaRepository.h"
#include "Product.h"

/* @Repository */
DefineStandardPointers(ProductRepository)
class ProductRepository : public CpaRepository<Product, int> {
    Public Virtual ~ProductRepository() = default;
    
    // Custom FindBy methods
    Public Virtual optional<Product> FindByName(StdString name) = 0;
    Public Virtual StdVector<Product> FindByCategory(StdString category) = 0;
};

#endif // _PRODUCT_REPOSITORY_H_

