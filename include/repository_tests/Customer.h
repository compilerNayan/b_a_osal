#ifndef CUSTOMER_H
#define CUSTOMER_H

#include <StandardDefines.h>

/* @Entity */
class Customer {

    /* @Id */
    /* @NotNull */
    Public optional<int> id;

    Public optional<StdString> email;

    Public optional<StdString> firstName;

    Public optional<StdString> lastName;

    Public optional<StdString> phone;

};

#endif

