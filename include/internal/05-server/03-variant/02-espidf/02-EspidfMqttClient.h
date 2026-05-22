#ifndef ESPIDF_MQTT_CLIENT_INTERNAL_H
#define ESPIDF_MQTT_CLIENT_INTERNAL_H

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
#include "../../02-interface/03-IAwsIotCoreConfigProvider.h"

/* @Component */
class EspidfMqttClient final : public IMqttClient {

    /* @Autowired */
    Private IAwsIotCoreConfigProviderPtr configProvider;
    /* @Autowired */
    Private ILoggerPtr logger;

    Private esp_mqtt_client_handle_t client;
    Private Bool running;

    // Broker config strings
    Private StdString brokerUri;
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

    Private Static StdString BuildMqttUri(CStdString& endpoint) {
        if (endpoint.find("://") != StdString::npos) {
            return endpoint;
        }
        return "mqtts://" + endpoint + ":8883";
    }

    Private Static Void MqttEventHandler(Void* handler_args,
                                        esp_event_base_t base,
                                        Int32 event_id,
                                        Void* event_data) {
        EspidfMqttClient* client = static_cast<EspidfMqttClient*>(handler_args);
        esp_mqtt_event_handle_t event = (esp_mqtt_event_handle_t)event_data;

        switch (event->event_id) {
            case MQTT_EVENT_CONNECTED: {
                client->logger->Info(Tag::Untagged, "MQTT connection established");
                client->running = true;
                break;
            }
            case MQTT_EVENT_DISCONNECTED: {
                client->logger->Warning(Tag::Untagged, "MQTT disconnected from broker");
                client->running = false;
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
                client->logger->Info(Tag::Untagged,
                    "Publish acknowledged msg_id=" + std::to_string(event->msg_id));
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

                client->logger->Info(Tag::Untagged,
                    "Buffered message GUID=" + msg.guid +
                    " topic=" + topicx +
                    " payload=" + msg.payload);
                break;
            }
            case MQTT_EVENT_ERROR: {
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

    Public Explicit EspidfMqttClient()
        : configProvider(Implementation<IAwsIotCoreConfigProvider>::type::GetInstance()),
          logger(Implementation<ILogger>::type::GetInstance()),
          client(nullptr),
          running(false) {}

    Public Virtual ~EspidfMqttClient() override {
        Disconnect();
    }

    Public Virtual Bool Connect() override {
        if (running) return true;

        StdString endpoint = configProvider->GetEndpoint();
        brokerUri = BuildMqttUri(endpoint);
        clientId = configProvider->GetThingName();
        caCert = configProvider->GetCaCert();
        deviceCert = configProvider->GetDeviceCert();
        privateKey = configProvider->GetPrivateKey();

        esp_mqtt_client_config_t mqtt_cfg = {};
        mqtt_cfg.broker.address.uri = brokerUri.c_str();
        mqtt_cfg.broker.verification.certificate = caCert.c_str();
        mqtt_cfg.credentials.client_id = clientId.c_str();
        mqtt_cfg.credentials.authentication.certificate = deviceCert.c_str();
        mqtt_cfg.credentials.authentication.key = privateKey.c_str();

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

        logger->Info(Tag::Untagged, "AWS IoT Core client started uri=" + brokerUri);
        return true;
    }

    Public Virtual Bool Disconnect() override {
        if (client) {
            esp_mqtt_client_stop(client);
            esp_mqtt_client_destroy(client);
            client = nullptr;
        }
        running = false;
        logger->Info(Tag::Untagged, "AWS IoT Core client stopped");
        return true;
    }

    Public Virtual Bool IsConnected() const override {
        return running;
    }

    Public Virtual Bool RefreshConnection() override {
        Disconnect();
        return Connect();
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
        } else {
            logger->Info(Tag::Untagged,
                "Published GUID=" + msg.guid + " topic=" + topic +
                " payload=" + msg.payload);
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
