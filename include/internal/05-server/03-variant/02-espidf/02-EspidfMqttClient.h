#ifndef ESPIDF_MQTT_CLIENT_INTERNAL_H
#define ESPIDF_MQTT_CLIENT_INTERNAL_H

#include <atomic>
#include <cstddef>
#include "esp_heap_caps.h"
#include <esp_event.h>
#include <mqtt_client.h>
#include <string>
#include <optional>
#include <deque>
#include <unordered_map>
#include <unordered_set>
#include <mutex>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include <StandardDefines.h>

#include "logger/ILogger.h"
#include "util/GuidUtil.h"

#include "../../02-interface/02-IMqttClient.h"
#include "../../data/DeviceIdentityProfileData.h"

/* @Component */
class EspidfMqttClient final : public IMqttClient {

    /* @Autowired */
    Private ILoggerPtr logger;

    Private esp_mqtt_client_handle_t client;
    Private Bool running;
    Private std::atomic<bool> connectAttemptFailed_{false};

    // Broker config strings
    Private StdString brokerUri;
    Private StdString brokerHost;
    Private StdString clientId;
    Private StdString caCert;
    Private StdString deviceCert;
    Private StdString privateKey;

    // Buffers
    Private StdUnorderedMap<StdString, StdDeque<MqttMessage>> receiveBuffer_;
    Private StdDeque<std::pair<StdString, MqttMessage>> sendBuffer_;

    // Mutexes for coarse-grained locking
    Private mutable std::mutex receiveMutex_;
    Private mutable std::mutex sendMutex_;
    Private mutable std::mutex subscriptionMutex_;

    struct HeapSnapshot {
        size_t freeBytes;
        size_t minFreeBytes;
        size_t largestBlock;

        static HeapSnapshot Capture() {
            const UInt32 caps = MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT;
            return {
                esp_get_free_heap_size(),
                esp_get_minimum_free_heap_size(),
                heap_caps_get_largest_free_block(caps)
            };
        }
    };

    Private Void LogRefreshHeap(const char* label, const HeapSnapshot& snap) {
        logger->Info(Tag::Untagged,
            StdString("RefreshConnection heap ") + label +
            " heap_free=" + std::to_string(snap.freeBytes) +
            " heap_min=" + std::to_string(snap.minFreeBytes) +
            " largest_block=" + std::to_string(snap.largestBlock));
    }

    Private Void LogRefreshHeapDelta(const HeapSnapshot& before, const HeapSnapshot& after) {
        const long freeDelta =
            static_cast<long>(after.freeBytes) - static_cast<long>(before.freeBytes);
        const long minDelta =
            static_cast<long>(after.minFreeBytes) - static_cast<long>(before.minFreeBytes);
        const long largestDelta =
            static_cast<long>(after.largestBlock) - static_cast<long>(before.largestBlock);
        logger->Info(Tag::Untagged,
            "RefreshConnection heap delta heap_free=" + std::to_string(freeDelta) +
            " heap_min=" + std::to_string(minDelta) +
            " largest_block=" + std::to_string(largestDelta));
    }

    /** Ensures mqtts URI includes explicit port (ESP-TLS parser requires it for AWS IoT). */
    Private Static StdString NormalizeMqttUri(const StdString& endpoint) {
        StdString uri = endpoint;
        if (uri.find("://") == StdString::npos) {
            return "mqtts://" + uri + ":8883";
        }
        const size_t hostStart = uri.find("://") + 3;
        const size_t slash = uri.find('/', hostStart);
        const size_t colon = uri.find(':', hostStart);
        const Bool hasPort = (colon != StdString::npos && (slash == StdString::npos || colon < slash));
        if (!hasPort) {
            if (slash != StdString::npos) {
                uri.insert(slash, ":8883");
            } else {
                uri += ":8883";
            }
        }
        return uri;
    }

    Private Static Void ParseBrokerHostPort(const StdString& uri, StdString& host, Int& port) {
        port = 8883;
        host.clear();
        const size_t scheme = uri.find("://");
        const size_t hostStart = (scheme == StdString::npos) ? 0 : scheme + 3;
        const size_t colon = uri.find(':', hostStart);
        const size_t slash = uri.find('/', hostStart);
        if (colon != StdString::npos && (slash == StdString::npos || colon < slash)) {
            host = uri.substr(hostStart, colon - hostStart);
            port = atoi(uri.c_str() + colon + 1);
        } else if (slash != StdString::npos) {
            host = uri.substr(hostStart, slash - hostStart);
        } else {
            host = uri.substr(hostStart);
        }
    }

    Private Static Void MqttEventHandler(Void* handler_args,
                                        esp_event_base_t base,
                                        Int32 event_id,
                                        Void* event_data) {
        EspidfMqttClient* client = static_cast<EspidfMqttClient*>(handler_args);
        esp_mqtt_event_handle_t event = (esp_mqtt_event_handle_t)event_data;

        switch (event->event_id) {
            case MQTT_EVENT_CONNECTED: {
                client->connectAttemptFailed_ = false;
                client->logger->Info(Tag::Untagged, "MQTT connection established");
                client->running = true;
                break;
            }
            case MQTT_EVENT_DISCONNECTED: {
                client->logger->Warning(Tag::Untagged, "MQTT disconnected from broker");
                client->running = false;
                client->connectAttemptFailed_ = true;
                break;
            }
            case MQTT_EVENT_SUBSCRIBED: {
                StdString topic(event->topic, event->topic_len);
                client->logger->Info(Tag::Untagged,
                    "Broker acknowledged subscription to topic=" + topic +
                    " msg_id=" + std::to_string(event->msg_id));
                break;
            }
            case MQTT_EVENT_UNSUBSCRIBED: {
                StdString topic(event->topic, event->topic_len);
                client->logger->Info(Tag::Untagged,
                    "Broker acknowledged unsubscription from topic=" + topic +
                    " msg_id=" + std::to_string(event->msg_id));
                break;
            }
            case MQTT_EVENT_PUBLISHED: {
                //client->logger->Info(Tag::Untagged,
                //    "Publish acknowledged msg_id=" + std::to_string(event->msg_id));
                break;
            }
            case MQTT_EVENT_DATA: {
                StdString topicx(event->topic, event->topic_len);
                MqttMessage msg;
                msg.guid = GuidUtil::GenerateGuid();
                msg.payload = StdString(event->data, event->data_len);

                {
                    std::lock_guard<std::mutex> lock(client->receiveMutex_);
                    client->receiveBuffer_[topicx].push_back(msg);
                }

                break;
            }
            case MQTT_EVENT_ERROR: {
                client->running = false;
                client->connectAttemptFailed_ = true;
                client->logger->Error(Tag::Untagged, "MQTT_EVENT_ERROR occurred");
                if (event->error_handle) {
                    auto err = event->error_handle;
                    client->logger->Error(Tag::Untagged,
                        "Error type=" + std::to_string(err->error_type) +
                        " tls_last_esp_err=" + std::to_string(err->esp_tls_last_esp_err) +
                        " tls_stack_err=" + std::to_string(err->esp_tls_stack_err) +
                        " transport_sock_errno=" + std::to_string(err->esp_transport_sock_errno));
                }
                break;
            }
            case MQTT_EVENT_BEFORE_CONNECT: {
                client->logger->Info(Tag::Untagged, "MQTT_EVENT_BEFORE_CONNECT: preparing to connect");
                break;
            }
            default: {
                client->logger->Info(Tag::Untagged,
                    "Unhandled MQTT event_id=" + std::to_string(event->event_id));
                break;
            }
        }
    }

    Public Explicit EspidfMqttClient() : 
          client(nullptr),
          running(false) {}

    Public Virtual ~EspidfMqttClient() override {
        Disconnect();
    }

    Public Virtual Bool Connect(const DeviceIdentityProfileData& deviceIdentityProfile) override {
        if (running) {
            return true;
        }
        if (client != nullptr) {
            Disconnect();
        }
        this->brokerUri = NormalizeMqttUri(deviceIdentityProfile.mqttEndpoint);
        Int brokerPort = 8883;
        ParseBrokerHostPort(this->brokerUri, this->brokerHost, brokerPort);
        this->clientId = deviceIdentityProfile.thingName;
        this->caCert = deviceIdentityProfile.caCertificatePem;
        this->deviceCert = deviceIdentityProfile.clientCertificatePem;
        this->privateKey = deviceIdentityProfile.clientPrivateKeyPem;

        connectAttemptFailed_ = false;

        esp_mqtt_client_config_t mqtt_cfg = {};
        mqtt_cfg.broker.address.hostname = this->brokerHost.c_str();
        mqtt_cfg.broker.address.port = brokerPort;
        mqtt_cfg.broker.address.transport = MQTT_TRANSPORT_OVER_SSL;
        mqtt_cfg.broker.verification.certificate = this->caCert.c_str();
        mqtt_cfg.credentials.client_id = this->clientId.c_str();
        mqtt_cfg.credentials.authentication.certificate = this->deviceCert.c_str();
        mqtt_cfg.credentials.authentication.key = this->privateKey.c_str();
        // Reconnect only when MqttClientManager calls Connect/Refresh (internet must be up).
        mqtt_cfg.network.disable_auto_reconnect = true;

        client = esp_mqtt_client_init(&mqtt_cfg);
        if (!client) {
            logger->Error(Tag::Untagged, "MQTT client init failed for uri=" + brokerUri);
            return false;
        }

        esp_mqtt_client_register_event(client, MQTT_EVENT_ANY, MqttEventHandler, this);
        if (esp_mqtt_client_start(client) != ESP_OK) {
            logger->Error(Tag::Untagged, "MQTT client start failed for uri=" + brokerUri);
            esp_mqtt_client_destroy(client);
            client = nullptr;
            return false;
        }

        logger->Info(Tag::Untagged,
            "AWS IoT Core client started host=" + brokerHost + " port=" + std::to_string(brokerPort));
        return true;
    }

    Public Virtual Bool IsClientStarted() const override {
        return client != nullptr;
    }

    Public Virtual Bool Disconnect() override {
        if (!client) {
            running = false;
            return true;
        }
        esp_mqtt_client_stop(client);
        esp_mqtt_client_destroy(client);
        client = nullptr;
        running = false;
        logger->Info(Tag::Untagged, "AWS IoT Core client stopped");
        return true;
    }

    Public Virtual Bool IsConnected() const override {
        return running;
    }

    Public Virtual Bool RefreshConnection(const DeviceIdentityProfileData& deviceIdentityProfile) override {
        const HeapSnapshot heapBefore = HeapSnapshot::Capture();
        LogRefreshHeap("before", heapBefore);

        Disconnect();
        Thread::Sleep(2000);
        const Bool connected = Connect(deviceIdentityProfile);

        const HeapSnapshot heapAfter = HeapSnapshot::Capture();
        LogRefreshHeap("after", heapAfter);
        LogRefreshHeapDelta(heapBefore, heapAfter);

        return connected;
    }

    Public Virtual Bool WaitForConnection(Int timeoutMs) override {
        if (running) return true;

        const Int intervalMs = 100;
        Int waited = 0;

        while (waited < timeoutMs) {
            if (IsConnected()) {
                logger->Info(Tag::Untagged,
                    "MQTT connection established after " + std::to_string(waited) + "ms");
                return true;
            }
            if (connectAttemptFailed_) {
                logger->Warning(Tag::Untagged,
                    "MQTT connect attempt failed after " + std::to_string(waited) + "ms");
                return false;
            }
            vTaskDelay(pdMS_TO_TICKS(intervalMs));
            waited += intervalMs;
        }

        logger->Warning(Tag::Untagged,
            "MQTT connection not established within timeout=" + std::to_string(timeoutMs) + "ms");
        return false;
    }

    Public Virtual Bool Subscribe(CStdString& topic) override {
        Int id = esp_mqtt_client_subscribe(client, topic.c_str(), 1);
        if (id != -1) {
            logger->Info(Tag::Untagged,
                "Sent SUBSCRIBE packet for topic=" + topic +
                " msg_id=" + std::to_string(id));
            return true;
        } else {
            logger->Warning(Tag::Untagged,
                "Failed to send SUBSCRIBE for topic=" + topic);
            return false;
        }
    }

    // SendMessage drains one from send buffer if available
    Public Virtual Void SendMessage() override {
        if (!running || !client) return;
        std::pair<StdString, MqttMessage> entry;
        {
            std::lock_guard<std::mutex> lock(sendMutex_);
            if (sendBuffer_.empty()) return;
            entry = sendBuffer_.front();
            sendBuffer_.pop_front();
        }

        auto& topic = entry.first;
        auto& msg = entry.second;

        Int id = esp_mqtt_client_publish(client,
                                         topic.c_str(),
                                         msg.payload.c_str(),
                                         static_cast<Int>(msg.payload.size()),
                                         1,
                                         0);
        if (id == -1) {
            logger->Error(Tag::Untagged,
                "Publish failed GUID=" + msg.guid + " topic=" + topic);
        }
    }

    // Application-facing buffer access
    Public Virtual Optional<MqttMessage> GetNextReceivedMessage(CStdString& topic) override {
        std::lock_guard<std::mutex> lock(receiveMutex_);
        auto it = receiveBuffer_.find(topic);
        if (it == receiveBuffer_.end() || it->second.empty()) return std::nullopt;

        auto msg = it->second.front();
        it->second.pop_front();
        return msg;
    }

    Public Virtual Bool QueueMessageToSend(CStdString& topic, const MqttMessage& msg) override {
        std::lock_guard<std::mutex> lock(sendMutex_);
        sendBuffer_.push_back({topic, msg});
        return true;
    }

    Public Virtual Size GetPendingReceivedCount(CStdString& topic) const override {
        std::lock_guard<std::mutex> lock(receiveMutex_);
        auto it = receiveBuffer_.find(topic);
        if (it == receiveBuffer_.end()) return 0;
        return it->second.size();
    }

    Public Virtual Size GetPendingSendCount() const override {
        std::lock_guard<std::mutex> lock(sendMutex_);
        return sendBuffer_.size();
    }
};

#endif // ESPIDF_MQTT_CLIENT_INTERNAL_H
