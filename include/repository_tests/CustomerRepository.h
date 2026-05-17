#ifndef _CUSTOMER_REPOSITORY_H_
#define _CUSTOMER_REPOSITORY_H_

#include <StandardDefines.h>
#include "CpaRepository.h"
#include "Customer.h"

/* @Repository */
DefineStandardPointers(CustomerRepository)
class CustomerRepository : public CpaRepository<Customer, int> {
    Public Virtual ~CustomerRepository() = default;
    
    // Custom FindBy methods
    Public Virtual optional<Customer> FindByEmail(StdString email) = 0;
    Public Virtual StdVector<Customer> FindByLastName(StdString lastName) = 0;
    Public Virtual optional<Customer> FindByPhone(StdString phone) = 0;
};

#endif // _CUSTOMER_REPOSITORY_H_

