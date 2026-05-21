#ifdef ESP_PLATFORM
#ifndef ESPIDF_CLOCK_SYNCHRONIZER_H
#define ESPIDF_CLOCK_SYNCHRONIZER_H

#include "esp_sntp.h"
#include <time.h>

#include <StandardDefines.h>
#include <logger/ILogger.h>
#include "../../01-interface/01-IClockSynchronizer.h"

/* @Component */
class EspidfClockSynchronizer final : public IClockSynchronizer {

    Private time_t lastSyncTime;
    Private StdString timezone;   // store current timezone string

    /* @Autowired */
    Private ILoggerPtr logger;

    // Track active instance for static callback
    Private Static EspidfClockSynchronizer* activeInstance;

    Public EspidfClockSynchronizer() : lastSyncTime(0), timezone("IST-5:30") {}
    Public Virtual ~EspidfClockSynchronizer() override = default;

    Public Bool SyncIfNeeded(CStdString ntpServer,
                             Int timeoutMs,
                             Int intervalMs,
                             CStdString tz) override {
        time_t now;
        time(&now);

        // Skip if last sync was within 1 hour
        if (lastSyncTime != 0 && difftime(now, lastSyncTime) < 3600) {
            return true;
        }

        // Stop SNTP if already running
        esp_sntp_stop();

        // Save timezone for callback use
        timezone = tz;

        // Register static callback
        activeInstance = this;
        esp_sntp_set_time_sync_notification_cb(&EspidfClockSynchronizer::TimeSyncCallback);

        // Configure SNTP
        esp_sntp_setoperatingmode(SNTP_OPMODE_POLL);
        esp_sntp_setservername(0, ntpServer.c_str());

        // Start SNTP
        esp_sntp_init();

        logger->Info(Tag::Untagged, "Starting SNTP sync with server: " + ntpServer);

        Int elapsed = 0;
        while (lastSyncTime == 0 && elapsed < timeoutMs) {
            logger->Info(Tag::Untagged,
                "Waiting for system time... (" + std::to_string(elapsed) + "/" + std::to_string(timeoutMs) + " ms)");
            vTaskDelay(pdMS_TO_TICKS(intervalMs));
            elapsed += intervalMs;
        }

        if (lastSyncTime != 0) {
            time(&now);
            struct tm timeinfo;
            localtime_r(&now, &timeinfo);
            logger->Info(Tag::Untagged, "System time synced (" + timezone + "): " + StdString(asctime(&timeinfo)));
            return true;
        } else {
            logger->Warning(Tag::Untagged,
                "SNTP sync failed (timeout after " + std::to_string(timeoutMs) + " ms)");
            return false;
        }
    }

    Public time_t GetLastSyncTime() const override {
        return lastSyncTime;
    }

    Private Static Void TimeSyncCallback(struct timeval *tv) {
        if (activeInstance) {
            time_t now;
            time(&now);
            activeInstance->lastSyncTime = now;

            // ✅ Apply timezone here when SNTP actually sets system time
            setenv("TZ", activeInstance->timezone.c_str(), 1);
            tzset();

            struct tm timeinfo;
            localtime_r(&now, &timeinfo);
            activeInstance->logger->Info(Tag::Untagged,
                "Time sync callback: system time updated (" + activeInstance->timezone + ") "
                + StdString(asctime(&timeinfo)));
        }
    }
};

// Define static member
inline EspidfClockSynchronizer* EspidfClockSynchronizer::activeInstance = nullptr;

#endif // ESPIDF_CLOCK_SYNCHRONIZER_H
#endif // ESP_PLATFORM
