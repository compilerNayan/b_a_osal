#ifndef RETDTO_H
#define RETDTO_H

#include <StandardDefines.h>

/* @Serializable */
class RetDto {
public:
    optional<int> a;
    optional<int> b;
    optional<int> c;
};

#endif