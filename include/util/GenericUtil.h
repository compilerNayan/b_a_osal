#ifndef GENERIC_UTIL_H
#define GENERIC_UTIL_H

#include <StandardDefines.h>
#include "esp_system.h"  // for esp_random()

class GenericUtil {

    Public Static ULong GenerateConnectionId() {
        // esp_random() returns uint32_t
        uint32_t r = esp_random();
        // constrain to positive range (1 .. 2^31-1)
        return static_cast<ULong>(1 + (r % 2147483647));
    }
    
};

#endif // GENERIC_UTIL_H