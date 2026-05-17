#ifndef TESTDTO_H
#define TESTDTO_H

#include <StandardDefines.h>

/* @Entity */
class TestDto {

    /* @Id */
    int xx;

    Public optional<int> a;

    optional<int> b;

    optional<StdString> c;
};

#endif