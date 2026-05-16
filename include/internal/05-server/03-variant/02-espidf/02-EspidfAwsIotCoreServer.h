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
#include "util/GuidUtil.h"
#include "../../02-interface/01-IServer.h"
#include "../../02-interface/02-IAwsIotCoreConfigProvider.h"

class EspidfAwsIotCoreServer final : public IServer {
    /* @Autowired */
    Private IAwsIotCoreConfigProviderPtr configProvider;

    /* @Autowired */
    Private ILoggerPtr logger;

    Private esp_mqtt_client_handle_t client;
    Private Bool running;

    // Keep broker config strings alive for the MQTT client lifetime
    Private StdString brokerUri;
    Private StdString clientId;
    Private StdString caCert;
    Private StdString deviceCert;
    Private StdString privateKey;

    Private StdUnorderedMap<StdString, StdDeque<IoTMessage>> bufferedMessages;

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
        EspidfAwsIotCoreServer* server = static_cast<EspidfAwsIotCoreServer*>(handler_args);
        esp_mqtt_event_handle_t event = (esp_mqtt_event_handle_t)event_data;

        if (event->event_id == MQTT_EVENT_DATA) {
            IoTMessage msg;
            msg.guid = GuidUtil::GenerateGuid();
            msg.payload = StdString(event->data, event->data_len);
            StdString topicx = StdString(event->topic, event->topic_len);

            server->bufferedMessages[topicx].push_back(msg);
            server->logger->Info(Tag::Untagged,
                "Buffered message GUID=" + msg.guid +
                " topic=" + topicx +
                " payload=" + msg.payload);
        }
    }

    Public Explicit EspidfAwsIotCoreServer()
        : configProvider(Implementation<IAwsIotCoreConfigProvider>::type::GetInstance()),
          logger(Implementation<ILogger>::type::GetInstance()),
          client(nullptr),
          running(false) {}

    Public Virtual ~EspidfAwsIotCoreServer() override {
        Stop();
    }

    Public Bool Start() override {
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
            logger->Error(Tag::Untagged,
                "MQTT client init failed for uri=" + brokerUri);
            return false;
        }

        esp_mqtt_client_register_event(client, MQTT_EVENT_ANY,
                                       MqttEventHandler, this);
        if (esp_mqtt_client_start(client) != ESP_OK) {
            logger->Error(Tag::Untagged,
                "MQTT client start failed for uri=" + brokerUri);
            esp_mqtt_client_destroy(client);
            client = nullptr;
            return false;
        }

        running = true;
        logger->Info(Tag::Untagged,
            "AWS IoT Core server started uri=" + brokerUri);
        return true;
    }

    Public Bool Stop() override {
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

    Public Bool IsRunning() const override {
        return running;
    }

    Public Bool Restart() override {
        Stop();
        return Start();
    }

    Public Optional<IoTMessage> ReceiveMessage(Optional<StdString> receiveTopic) override {
        if (!receiveTopic.has_value()) {
            logger->Error(Tag::Untagged, "ReceiveMessage called without a receive topic");
            return std::nullopt;
        }

        if (!running) return std::nullopt;

        if (bufferedMessages.find(receiveTopic.value()) == bufferedMessages.end()) {
            esp_mqtt_client_subscribe(client, receiveTopic.value().c_str(), 1);
            logger->Info(Tag::Untagged, "Subscribed to new topic=" + receiveTopic.value());
            return std::nullopt;
        }

        auto& queue = bufferedMessages[receiveTopic.value()];
        if (queue.empty()) return std::nullopt;

        IoTMessage msg = queue.front();
        queue.pop_front();
        logger->Info(Tag::Untagged,
            "Delivering GUID=" + msg.guid +
            " topic=" + receiveTopic.value() +
            " payload=" + msg.payload);
        return msg;
    }

    Public Bool SendMessage(const IoTMessage& msg, Optional<StdString> sendTopic) override {
        if (!sendTopic.has_value()) {
            logger->Error(Tag::Untagged, "SendMessage called without a send topic");
            return false;
        }
        StdString topic = sendTopic.value();
        if (!running || !client) {
            logger->Warning(Tag::Untagged, "SendMessage called but server not running");
            return false;
        }
        Int id = esp_mqtt_client_publish(client,
                                         topic.c_str(),
                                         msg.payload.c_str(),
                                         static_cast<Int>(msg.payload.size()),
                                         1,
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
