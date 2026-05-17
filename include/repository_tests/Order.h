#ifndef ORDER_H
#define ORDER_H

#include <StandardDefines.h>

/* @Entity */
class Order {

    /* @Id */
    /* @NotNull */
    Public optional<int> id;

    Public optional<StdString> orderNumber;

    Public optional<int> customerId;

    Public optional<double> totalAmount;
};

#endif

