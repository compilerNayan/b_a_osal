#ifndef ESPIDF_AWS_IOT_CORE_SERVER_INTERNAL_H
#define ESPIDF_AWS_IOT_CORE_SERVER_INTERNAL_H

#include <esp_event.h>
#include <mqtt_client.h>
#include <string>
#include <optional>
#include <unordered_map>
#include <vector>

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
    Private StdUnorderedMap<StdString, StdVector<StdString>> bufferedMessages;

    Private Static Void MqttEventHandler(Void* handler_args,
                                         esp_event_base_t base,
                                         Int32 event_id,
                                         Void* event_data) {
        EspidfAwsIotCoreServer* server = static_cast<EspidfAwsIotCoreServer*>(handler_args);
        esp_mqtt_event_handle_t event = (esp_mqtt_event_handle_t)event_data;

        if (event->event_id == MQTT_EVENT_DATA) {
            StdString topic(event->topic, event->topic_len);
            StdString payload(event->data, event->data_len);
            server->bufferedMessages[topic].push_back(payload);
            server->logger->Info(Tag::Untagged,
                "Received message topic=" + topic + " payload=" + payload);
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

    Public Virtual Optional<IoTMessage> ReceiveMessage(Optional<StdString> receiveTopic) override {
        if (!running) return std::nullopt;
        if (bufferedMessages.empty()) return std::nullopt;

        Var it = bufferedMessages.begin();
        if (it->second.empty()) return std::nullopt;

        IoTMessage msg;
        msg.guid = "guid-" + std::to_string(esp_random());
        msg.payload = it->second.front();
        it->second.erase(it->second.begin());
        if (it->second.empty()) bufferedMessages.erase(it);

        logger->Info(Tag::Untagged,
            "Delivering GUID=" + msg.guid +
            " topic=" + msg.address.value_or("") +
            " payload=" + msg.payload);

        return msg;
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
        StdString topic = msg.address.value_or(configProvider->GetSendTopic());
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