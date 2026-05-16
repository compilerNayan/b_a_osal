#ifndef GUIDUTIL_H
#define GUIDUTIL_H

#include "StandardDefines.h"
#include <esp_timer.h>
#include <esp_system.h>

class GuidUtil {
    Public Static StdString GenerateGuid() {
        // Simple pseudo-GUID using timestamp + random
        char buf[64];
        snprintf(buf, sizeof(buf), "%lu-%lu",
                (unsigned long)esp_timer_get_time(),
                (unsigned long)esp_random());
        return StdString(buf);
    }
};

#endif // GUIDUTIL_H
