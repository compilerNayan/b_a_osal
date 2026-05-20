#ifdef ESP_PLATFORM
#ifndef ESPIDF_WIFI_MANAGER_INTERNAL_H
#define ESPIDF_WIFI_MANAGER_INTERNAL_H

#include <StandardDefines.h>
#include "../../01-interface/01-IWiFiManager.h"

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "esp_netif.h"

#include "logger/ILogger.h"

/* @Component */
class EspidfWiFiManager : public IWiFiManager {
    Private EventGroupHandle_t wifiEventGroup;
    Private StdString ssid;
    Private StdString password;
    Private WiFiConnectionStatus status;

    /* @Autowired */
    Private ILoggerPtr logger;   

    Private Static Const Int WIFI_CONNECTED_BIT = BIT0;

    Private Static Void EventHandler(VoidPtr arg, esp_event_base_t event_base,
                                    int32_t event_id, VoidPtr event_data) {
        EspidfWiFiManager* client = static_cast<EspidfWiFiManager*>(arg);

        if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
            client->status = WiFiConnectionStatus::Connecting;
            client->logger->Info(Tag::Untagged, "[EspidfWiFiManager] STA started, attempting connect...");
            esp_wifi_connect();
        } 
        else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
            wifi_event_sta_disconnected_t* disconn = (wifi_event_sta_disconnected_t*)event_data;
            client->status = WiFiConnectionStatus::Failed;

            switch (disconn->reason) {
                case WIFI_REASON_AUTH_FAIL:
                case WIFI_REASON_HANDSHAKE_TIMEOUT:
                    client->logger->Error(Tag::Untagged, "[EspidfWiFiManager] Authentication failed (wrong password?)");
                    break;
                case WIFI_REASON_NO_AP_FOUND:
                    client->logger->Error(Tag::Untagged, "[EspidfWiFiManager] SSID not found");
                    break;
                case WIFI_REASON_ASSOC_LEAVE:
                    client->logger->Warning(Tag::Untagged, "[EspidfWiFiManager] Disconnected by AP");
                    break;
                case WIFI_REASON_BEACON_TIMEOUT:
                    client->logger->Warning(Tag::Untagged, "[EspidfWiFiManager] Beacon timeout (AP not responding)");
                    break;
                default:
                    client->logger->Warning(Tag::Untagged, "[EspidfWiFiManager] Disconnected, reason=" + std::to_string(disconn->reason));
                    break;
            }

            // Do NOT auto-retry here. Let caller decide whether to reconnect.
        } 
        else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
            xEventGroupSetBits(client->wifiEventGroup, WIFI_CONNECTED_BIT);
            client->status = WiFiConnectionStatus::Connected;
            client->logger->Info(Tag::Untagged, "[EspidfWiFiManager] Got IP address, WiFi connected!");
        }
    }

    Private Bool InitNVS() {
        esp_err_t ret = nvs_flash_init();
        if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
            if (nvs_flash_erase() != ESP_OK) {
                logger->Error(Tag::Untagged, "[EspidfWiFiManager] NVS erase failed");
                return false;
            }
            ret = nvs_flash_init();
        }
        if (ret != ESP_OK) {
            logger->Error(Tag::Untagged, "[EspidfWiFiManager] NVS init failed: " + std::to_string(ret));
            return false;
        }
        return true;
    }

    Public EspidfWiFiManager() {
        wifiEventGroup = xEventGroupCreate();
        status = WiFiConnectionStatus::Disconnected;
        InitNVS(); // safe now, won’t crash
    }

    Public Virtual Bool Connect(CStdString ssid, const Optional<CStdString> password) override {
        this->ssid = ssid;
        this->password = password.has_value() ? password.value() : "";

        logger->Info(Tag::Untagged, "[EspidfWiFiManager] Starting WiFi connection to SSID: " + ssid);

        if (esp_netif_init() != ESP_OK) return false;
        if (esp_event_loop_create_default() != ESP_OK) return false;
        if (!esp_netif_create_default_wifi_sta()) return false;

        wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
        if (esp_wifi_init(&cfg) != ESP_OK) return false;
        if (esp_wifi_set_storage(WIFI_STORAGE_RAM) != ESP_OK) return false;

        if (esp_event_handler_instance_register(WIFI_EVENT,
                                                ESP_EVENT_ANY_ID,
                                                &EventHandler,
                                                this,
                                                nullptr) != ESP_OK) return false;
        if (esp_event_handler_instance_register(IP_EVENT,
                                                IP_EVENT_STA_GOT_IP,
                                                &EventHandler,
                                                this,
                                                nullptr) != ESP_OK) return false;

        wifi_config_t wifi_config = {};
        strncpy((Char*)wifi_config.sta.ssid, ssid.c_str(), sizeof(wifi_config.sta.ssid));
        strncpy((Char*)wifi_config.sta.password, this->password.c_str(), sizeof(wifi_config.sta.password));
        wifi_config.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;

        if (esp_wifi_set_mode(WIFI_MODE_STA) != ESP_OK) return false;
        if (esp_wifi_set_config(WIFI_IF_STA, &wifi_config) != ESP_OK) return false;
        if (esp_wifi_start() != ESP_OK) return false;

        status = WiFiConnectionStatus::Connecting;
        return true;
    }

    Public Virtual Void Disconnect() override {
        esp_err_t err = esp_wifi_disconnect();
        if (err == ESP_ERR_WIFI_NOT_INIT || err == ESP_ERR_WIFI_NOT_STARTED) {
            logger->Warning(Tag::Untagged, "[EspidfWiFiManager] Disconnect called but WiFi not running");
        } else if (err != ESP_OK) {
            logger->Error(Tag::Untagged, "[EspidfWiFiManager] esp_wifi_disconnect failed: " + std::to_string(err));
        }
        status = WiFiConnectionStatus::Disconnected;
        logger->Info(Tag::Untagged, "[EspidfWiFiManager] Disconnected from WiFi");
    }

    Public Virtual Bool IsConnected() const override {
        return status == WiFiConnectionStatus::Connected;
    }

    Public Virtual Bool WaitForConnection(Int timeoutMs) override {
        EventBits_t bits = xEventGroupWaitBits(
            wifiEventGroup,
            WIFI_CONNECTED_BIT,
            pdFALSE,
            pdFALSE,
            pdMS_TO_TICKS(timeoutMs)
        );

        if (bits & WIFI_CONNECTED_BIT) {
            return true;
        } else {
            status = WiFiConnectionStatus::Timeout;
            logger->Error(Tag::Untagged, "[EspidfWiFiManager] Connection timed out");
            return false;
        }
    }

    Public Virtual WiFiConnectionStatus GetStatus() const override {
        return status;
    }

    Public Virtual Optional<StdString> GetIPAddress() const override {
        esp_netif_t* netif = esp_netif_get_handle_from_ifkey("WIFI_STA_DEF");
        esp_netif_ip_info_t ip_info;
        if (esp_netif_get_ip_info(netif, &ip_info) == ESP_OK) {
            Char buf[16];
            sprintf(buf, IPSTR, IP2STR(&ip_info.ip));
            return StdString(buf);
        }
        return {};
    }

    Public Virtual StdString GetMACAddress() const override {
        uint8_t mac[6];
        if (esp_wifi_get_mac(WIFI_IF_STA, mac) != ESP_OK) {
            return "";
        }
        Char buf[18];
        sprintf(buf, "%02X:%02X:%02X:%02X:%02X:%02X",
                mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
        return StdString(buf);
    }

    Public Virtual StdVector<StdString> ScanNetworks() const override {
        StdVector<StdString> networks;
        uint16_t number = 20;
        wifi_ap_record_t ap_info[20];
        uint16_t ap_count = 0;
        memset(ap_info, 0, sizeof(ap_info));

        if (esp_wifi_scan_start(NULL, true) == ESP_OK &&
            esp_wifi_scan_get_ap_records(&number, ap_info) == ESP_OK) {
            ap_count = number;
            for (int i = 0; i < ap_count; i++) {
                networks.push_back(StdString(reinterpret_cast<Char*>(ap_info[i].ssid)));
            }
        } else {
            logger->Error(Tag::Untagged, "[EspidfWiFiManager] WiFi scan failed");
        }
        return networks;
    }
};

#endif // ESPIDF_WIFI_MANAGER_INTERNAL_H
#endif // ESP_PLATFORM
