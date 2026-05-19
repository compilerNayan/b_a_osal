#ifdef ESP_PLATFORM
#ifndef ESPIDF_HOTSPOT_MANAGER_INTERNAL_H
#define ESPIDF_HOTSPOT_MANAGER_INTERNAL_H

#include <StandardDefines.h>
#include "../../02-interface/02-IHotspotManager.h"

#include "freertos/FreeRTOS.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "esp_netif.h"

#include "logger/ILogger.h"

/* @Component */
class EspidfHotspotManager : public IHotspotManager {
    Private Bool active;
    Private Int connectedClients;
    Private StdString ssid;
    Private StdString password;

    /* @Autowired */
    Private ILoggerPtr logger;

    Private Static Void EventHandler(VoidPtr arg, esp_event_base_t event_base,
                                     int32_t event_id, VoidPtr event_data) {
        EspidfHotspotManager* manager = static_cast<EspidfHotspotManager*>(arg);

        if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_AP_START) {
            manager->active = true;
            manager->logger->Info(Tag::Untagged, "[EspidfHotspotManager] Hotspot started");
        } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_AP_STOP) {
            manager->active = false;
            manager->logger->Info(Tag::Untagged, "[EspidfHotspotManager] Hotspot stopped");
        } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_AP_STACONNECTED) {
            manager->connectedClients++;
            manager->logger->Info(Tag::Untagged, "[EspidfHotspotManager] Device connected (total=" + 
                                  std::to_string(manager->connectedClients) + ")");
        } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_AP_STADISCONNECTED) {
            manager->connectedClients--;
            manager->logger->Warning(Tag::Untagged, "[EspidfHotspotManager] Device disconnected (total=" + 
                                     std::to_string(manager->connectedClients) + ")");
        }
    }

    Private Bool InitNVS() {
        esp_err_t ret = nvs_flash_init();
        if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
            if (nvs_flash_erase() != ESP_OK) {
                logger->Error(Tag::Untagged, "[EspidfHotspotManager] NVS erase failed");
                return false;
            }
            ret = nvs_flash_init();
        }
        if (ret != ESP_OK) {
            logger->Error(Tag::Untagged, "[EspidfHotspotManager] NVS init failed: " + std::to_string(ret));
            return false;
        }
        return true;
    }

    Public EspidfHotspotManager() {
        active = false;
        connectedClients = 0;
        InitNVS();
    }

    Public Virtual Bool Start(CStdString ssid, const Optional<CStdString> password, Int maxClients = 4) override {
        this->ssid = ssid;
        this->password = password.has_value() ? password.value() : "";

        logger->Info(Tag::Untagged, "[EspidfHotspotManager] Starting hotspot with SSID: " + ssid);

        if (esp_netif_init() != ESP_OK) return false;
        if (esp_event_loop_create_default() != ESP_OK) return false;
        if (!esp_netif_create_default_wifi_ap()) return false;

        wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
        if (esp_wifi_init(&cfg) != ESP_OK) return false;

        if (esp_event_handler_instance_register(WIFI_EVENT,
                                                ESP_EVENT_ANY_ID,
                                                &EventHandler,
                                                this,
                                                nullptr) != ESP_OK) return false;

        wifi_config_t wifi_config = {};
        strncpy((Char*)wifi_config.ap.ssid, ssid.c_str(), sizeof(wifi_config.ap.ssid));
        wifi_config.ap.ssid_len = ssid.length();
        wifi_config.ap.max_connection = maxClients;

        if (this->password.empty()) {
            wifi_config.ap.authmode = WIFI_AUTH_OPEN;
        } else {
            strncpy((Char*)wifi_config.ap.password, this->password.c_str(), sizeof(wifi_config.ap.password));
            wifi_config.ap.authmode = WIFI_AUTH_WPA2_PSK;
        }

        if (esp_wifi_set_mode(WIFI_MODE_AP) != ESP_OK) return false;
        if (esp_wifi_set_config(WIFI_IF_AP, &wifi_config) != ESP_OK) return false;
        if (esp_wifi_start() != ESP_OK) return false;

        return true;
    }

    Public Virtual Bool Stop() override {
        esp_err_t err = esp_wifi_stop();
        if (err == ESP_ERR_WIFI_NOT_INIT || err == ESP_ERR_WIFI_NOT_STARTED) {
            logger->Warning(Tag::Untagged, "[EspidfHotspotManager] Stop called but hotspot not running");
        } else if (err != ESP_OK) {
            logger->Error(Tag::Untagged, "[EspidfHotspotManager] esp_wifi_stop failed: " + std::to_string(err));
            return false;
        }
        active = false;
        connectedClients = 0;
        logger->Info(Tag::Untagged, "[EspidfHotspotManager] Hotspot stopped");
        return true;
    }

    Public Virtual Bool IsActive() const override {
        return active;
    }

    Public Virtual Optional<StdString> GetIPAddress() const override {
        esp_netif_t* netif = esp_netif_get_handle_from_ifkey("WIFI_AP_DEF");
        esp_netif_ip_info_t ip_info;
        if (esp_netif_get_ip_info(netif, &ip_info) == ESP_OK) {
            Char buf[16];
            sprintf(buf, IPSTR, IP2STR(&ip_info.ip));
            return StdString(buf);
        }
        return {};
    }

    Public Virtual Int GetConnectedClients() const override {
        return connectedClients;
    }

    Public Virtual StdVector<StdString> ListClients() const override {
        StdVector<StdString> clients;
        wifi_sta_list_t sta_list;
        if (esp_wifi_ap_get_sta_list(&sta_list) == ESP_OK) {
            for (int i = 0; i < sta_list.num; i++) {
                Char buf[18];
                sprintf(buf, "%02X:%02X:%02X:%02X:%02X:%02X",
                        sta_list.sta[i].mac[0], sta_list.sta[i].mac[1],
                        sta_list.sta[i].mac[2], sta_list.sta[i].mac[3],
                        sta_list.sta[i].mac[4], sta_list.sta[i].mac[5]);
                clients.push_back(StdString(buf));
            }
        }
        return clients;
    }

    Public Virtual Bool ChangeCredentials(CStdString ssid, CStdString password) override {
        Stop();
        Optional<CStdString> pwd = password.empty() ? Optional<CStdString>{} : Optional<CStdString>(password);
        return Start(ssid, pwd);
    }
};

#endif // ESPIDF_HOTSPOT_MANAGER_INTERNAL_H
#endif // ESP_PLATFORM
