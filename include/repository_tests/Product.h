#ifndef PRODUCT_H
#define PRODUCT_H

#include <StandardDefines.h>

/* @Entity */
class Product {

    /* @Id */
    /* @NotNull */
    Public optional<int> id;

    Public optional<StdString> name;

    Public optional<double> price;

    Public optional<StdString> category;
};

#endif

