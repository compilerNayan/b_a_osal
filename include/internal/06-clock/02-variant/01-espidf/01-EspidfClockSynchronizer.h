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

    /* @Autowired */
    Private ILoggerPtr logger;

    Public EspidfClockSynchronizer() : lastSyncTime(0) {}
    Public Virtual ~EspidfClockSynchronizer() override = default;

    // Sync device clock with retries until timeout, but only if >1 hour since last sync
    Public Bool SyncIfNeeded(const Char* ntpServer = "pool.ntp.org", Int timeoutMs = 10000, Int intervalMs = 2000) override {
        time_t now;
        time(&now);

        // Check if last sync was within 1 hour
        if (lastSyncTime != 0 && difftime(now, lastSyncTime) < 3600) {
            logger->Info(Tag::Untagged, "Skipping sync (last sync was " + std::to_string(difftime(now, lastSyncTime)) + " seconds ago)");
            return true; // Consider still valid
        }

        // Configure SNTP
        esp_sntp_config_t config = ESP_NETIF_SNTP_DEFAULT_CONFIG(ntpServer);
        esp_netif_sntp_init(&config);

        logger->Info(Tag::Untagged, "Starting SNTP sync with server: " + std::string(ntpServer));

        Int elapsed = 0;
        while (sntp_get_sync_status() == SNTP_SYNC_STATUS_RESET && elapsed < timeoutMs) {
            logger->Info(Tag::Untagged, "Waiting for system time... (" + std::to_string(elapsed) + "/" + std::to_string(timeoutMs) + " ms)");
            vTaskDelay(pdMS_TO_TICKS(intervalMs));
            elapsed += intervalMs;
        }

        if (sntp_get_sync_status() == SNTP_SYNC_STATUS_COMPLETED) {
            time(&now);
            lastSyncTime = now;  // Update only on success
            struct tm timeinfo;
            localtime_r(&now, &timeinfo);
            logger->Info(Tag::Untagged, "System time synced: " + std::string(asctime(&timeinfo)));
            return true;
        } else {
            logger->Warning(Tag::Untagged, "SNTP sync failed (timeout after " + std::to_string(timeoutMs) + " ms)");
            return false;
        }
    }

    Public time_t GetLastSyncTime() const override {
        return lastSyncTime;
    }
};


#endif // ESPIDF_CLOCK_SYNCHRONIZER_H