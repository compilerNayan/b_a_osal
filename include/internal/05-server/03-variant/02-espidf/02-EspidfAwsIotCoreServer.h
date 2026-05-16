#ifndef ESPIDF_AWS_IOT_CORE_SERVER_INTERNAL_H
#define ESPIDF_AWS_IOT_CORE_SERVER_INTERNAL_H

#include <esp_event.h>
#include <mqtt_client.h>
#include <string>
#include <optional>
#include <deque>
#include <unordered_map>

#include <StandardDefines.h>

#include "logger/ILogger.h"
#include "../../02-interface/01-IServer.h"
#include "../../02-interface/02-IAwsIotCoreConfigProvider.h"

class EspidfAwsIotCoreServer final : public IServer {
    /* @Autowired */
    Private IAwsIotCoreConfigProviderPtr configProvider;

    /* @Autowired */
    Private ILoggerPtr logger;

    Private esp_mqtt_client_handle_t client;
    Private Bool running;

    // Buffer per topic
    Private StdUnorderedMap<StdString, StdDeque<IoTMessage>> bufferedMessages;

    Private Static Void MqttEventHandler(Void* handler_args,
                                         esp_event_base_t base,
                                         Int32 event_id,
                                         Void* event_data) {
        EspidfAwsIotCoreServer* server = static_cast<EspidfAwsIotCoreServer*>(handler_args);
        esp_mqtt_event_handle_t event = (esp_mqtt_event_handle_t)event_data;

        if (event->event_id == MQTT_EVENT_DATA) {
            IoTMessage msg;
            msg.guid = "guid-" + std::to_string(esp_random());
            msg.payload = StdString(event->data, event->data_len);
            msg.address = StdString(event->topic, event->topic_len);

            server->bufferedMessages[*msg.address].push_back(msg);
            server->logger->Info(Tag::Untagged,
                "Buffered message GUID=" + msg.guid +
                " topic=" + msg.address.value_or("") +
                " payload=" + msg.payload);
        }
    }

    Public Explicit EspidfAwsIotCoreServer()
        : client(nullptr),
          running(false) {}

    Public Virtual ~EspidfAwsIotCoreServer() override {
        Stop();
    }

    Public Virtual Bool Start() override {
        if (running) return true;

        esp_mqtt_client_config_t mqtt_cfg = {};
        mqtt_cfg.uri = configProvider->GetEndpoint().c_str();
        mqtt_cfg.client_id = configProvider->GetThingName().c_str();
        mqtt_cfg.cert_pem = configProvider->GetCaCert().c_str();
        mqtt_cfg.client_cert_pem = configProvider->GetDeviceCert().c_str();
        mqtt_cfg.client_key_pem = configProvider->GetPrivateKey().c_str();

        client = esp_mqtt_client_init(&mqtt_cfg);
        esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID,
                                       MqttEventHandler, this);
        esp_mqtt_client_start(client);

        running = true;
        logger->Info(Tag::Untagged, "AWS IoT Core server started");
        return true;
    }

    Public Virtual Bool Stop() override {
        if (!running) return false;
        if (client) {
            esp_mqtt_client_stop(client);
            esp_mqtt_client_destroy(client);
            client = nullptr;
        }
        running = false;
        logger->Info(Tag::Untagged, "AWS IoT Core server stopped");
        return true;
    }

    Public Virtual Bool IsRunning() const override {
        return running;
    }

    Public Virtual Bool Restart() override {
        Stop();
        return Start();
    }

    // Subscribe dynamically and return buffered messages
    Public Virtual Optional<IoTMessage> ReceiveMessage(Optional<StdString> receiveTopic = std::nullopt) override {
        if(!receiveTopic.has_value()) {
            logger->Error(Tag::Untagged, "ReceiveMessage called without a receive topic");
            return std::nullopt;
        }

        StdString topic = receiveTopic.value();
        if (!running) return std::nullopt;

        if (receiveTopic.has_value()) {
            // Subscribe if not already buffering this topic
            if (bufferedMessages.find(receiveTopic.value()) == bufferedMessages.end()) {
                esp_mqtt_client_subscribe(client, receiveTopic.value().c_str(), 1);
                logger->Info(Tag::Untagged, "Subscribed to new topic=" + receiveTopic.value());
                return std::nullopt; // nothing yet
            }

            // Return next buffered message for this topic
            auto& queue = bufferedMessages[receiveTopic.value()];
            if (queue.empty()) return std::nullopt;

            IoTMessage msg = queue.front();
            queue.pop_front();
            logger->Info(Tag::Untagged,
                "Delivering GUID=" + msg.guid +
                " topic=" + msg.address.value_or("") +
                " payload=" + msg.payload);
            return msg;
        }

        return std::nullopt;
    }

    Public Virtual Bool SendMessage(CIoTMessage& msg, Optional<StdString> sendTopic) override {
        if(!sendTopic.has_value()) {
            logger->Error(Tag::Untagged, "SendMessage called without a send topic");
            return false;
        }
        StdString topic = sendTopic.value();
        if (!running || !client) {
            logger->Warn(Tag::Untagged, "SendMessage called but server not running");
            return false;
        }
        Int id = esp_mqtt_client_publish(client,
                                         topic.c_str(),
                                         msg.payload.c_str(),
                                         msg.payload.size(),
                                         1, // QoS
                                         0);
        if (id == -1) {
            logger->Error(Tag::Untagged,
                "Publish failed GUID=" + msg.guid + " topic=" + topic);
            return false;
        }
        logger->Info(Tag::Untagged,
            "Published GUID=" + msg.guid + " topic=" + topic +
            " payload=" + msg.payload);
        return true;
    }
};

#endif // ESPIDF_AWS_IOT_CORE_SERVER_INTERNAL_H
