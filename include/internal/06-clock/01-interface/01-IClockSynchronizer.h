#ifndef ICLOCK_SYNCHRONIZER_INTERNAL_H
#define ICLOCK_SYNCHRONIZER_INTERNAL_H

#include <time.h>
#include <StandardDefines.h>

DefineStandardPointers(IClockSynchronizer)
class IClockSynchronizer {
    Public Virtual ~IClockSynchronizer() = default;

    // Try to sync device clock if needed (only if >1 hour since last sync).
    // Returns true if sync succeeded or skipped due to recent sync, false if sync failed.
    Public Virtual Bool SyncIfNeeded(CStdString ntpServer = "pool.ntp.org",
                                     Int timeoutMs = 10000,
                                     Int intervalMs = 2000,
                                     CStdString tz = "IST-5:30") = 0;

    // Get the last successful sync time (Unix timestamp).
    Public Virtual time_t GetLastSyncTime() const = 0;
};

#endif // ICLOCK_SYNCHRONIZER_INTERNAL_H
