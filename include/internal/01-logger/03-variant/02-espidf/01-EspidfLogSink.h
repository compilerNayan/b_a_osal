#ifdef ESP_PLATFORM
#ifndef ESPIDF_LOGSINK_H
#define ESPIDF_LOGSINK_H

#include "esp_timer.h" 

#include <StandardDefines.h>
#include "../../02-interface/02-ILogSink.h"
#include "logger/ILogBuffer.h"

/* @Component */
class EspidfLogSink final : public ILogSink {
    /* @Autowired */
    Private ILogBufferPtr logBuffer;

    Public EspidfLogSink() = default;

    Public Virtual ~EspidfLogSink() override = default;

    Public Virtual Void WriteLog(CStdString& message) override {
        time_t nowSec = time(nullptr);
        // Only use time() for key when NTP has synced (year 2001+); otherwise use millis() so publish can convert later
        const time_t kMinValidEpoch = 978307200;  // 2001-01-01 00:00:00 UTC
        Bool timeValid = (nowSec != (time_t)-1 && nowSec >= kMinValidEpoch);

        char timeBuf[24];
        if (nowSec != (time_t)-1 && nowSec > 0) {
            struct tm* t = localtime(&nowSec);
            if (t && strftime(timeBuf, sizeof(timeBuf), "%Y-%m-%d %H:%M:%S", t) > 0) { /* ok */ }
            else snprintf(timeBuf, sizeof(timeBuf), "(time?)");
        } else {
            snprintf(timeBuf, sizeof(timeBuf), "(no time)");
        }
        printf("[");
        printf(timeBuf);
        printf("] ");
        printf("%s", message.c_str());
        printf("\n");

        static ULong seqPerSec = 0;
        ULongLong key;
        if (timeValid) {
            key = (ULongLong)nowSec * 1000ULL + (ULong)(seqPerSec++ % 1000);
        } else {
            key = (ULongLong)esp_timer_get_time() * 1000ULL + (ULong)(seqPerSec++ % 1000);
        }
        logBuffer->AddLog(key, message);
    }
};

#endif // ESPIDF_LOGSINK_H
#endif // ESP_PLATFORM

