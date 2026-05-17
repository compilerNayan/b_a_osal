#ifndef _ORDER_REPOSITORY_H_
#define _ORDER_REPOSITORY_H_

#include <StandardDefines.h>
#include "CpaRepository.h"
#include "Order.h"

/* @Repository */
DefineStandardPointers(OrderRepository)
class OrderRepository : public CpaRepository<Order, int> {
    Public Virtual ~OrderRepository() = default;
    
    // Custom FindBy methods
    Public Virtual optional<Order> FindByOrderNumber(StdString orderNumber) = 0;
    Public Virtual StdVector<Order> FindByCustomerId(int customerId) = 0;
};

#endif // _ORDER_REPOSITORY_H_

