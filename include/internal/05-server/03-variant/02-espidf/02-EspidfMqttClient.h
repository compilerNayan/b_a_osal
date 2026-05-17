#ifndef ESPIDF_MQTT_CLIENT_INTERNAL_H
#define ESPIDF_MQTT_CLIENT_INTERNAL_H

#include <esp_event.h>
#include <mqtt_client.h>
#include <string>
#include <optional>
#include <deque>
#include <unordered_map>

#include <StandardDefines.h>


#include "logger/ILogger.h"
#include "util/GuidUtil.h"

#include "../../02-interface/02-IMqttClient.h"
#include "../../02-interface/03-IAwsIotCoreConfigProvider.h"


class EspidfMqttClient final : public IMMqttClient {

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

    Private StdUnorderedMap<StdString, StdDeque<MqttMessage>> bufferedMessages;

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
            MqttMessage msg;
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

    Public Explicit EspidfMqttClient()
        : configProvider(Implementation<IAwsIotCoreConfigProvider>::type::GetInstance()),
          logger(Implementation<ILogger>::type::GetInstance()),
          client(nullptr),
          running(false) {}

    Public Virtual ~EspidfMqttClient() override {
        Disconnect();
    }

    Public Bool Connect() override {
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

    Public Bool Disconnect() override {
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

    Public Bool IsConnected() const override {
        return running;
    }

    Public Bool RefreshConnection() override {
        Disconnect();
        return Connect();
    }

    Public Virtual Bool WaitForConnection(Int timeoutMs) override {
        if (running) {
            // Already connected
            return true;
        }
    
        const Int intervalMs = 100; // poll every 100ms
        Int waited = 0;
    
        while (waited < timeoutMs) {
            if (IsConnected()) {
                logger->Info(Tag::Untagged,
                    "MQTT connection established after " + std::to_string(waited) + "ms");
                return true;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(intervalMs));
            waited += intervalMs;
        }
    
        logger->Warning(Tag::Untagged,
            "MQTT connection not established within timeout=" + std::to_string(timeoutMs) + "ms");
        return false;
    }
    
    Public Optional<MqttMessage> ReceiveMessage(CStdString& topic) override {
        if (!running) return std::nullopt;

        if (bufferedMessages.find(receiveTopic.value()) == bufferedMessages.end()) {
            esp_mqtt_client_subscribe(client, receiveTopic.value().c_str(), 1);
            logger->Info(Tag::Untagged, "Subscribed to new topic=" + receiveTopic.value());
            return std::nullopt;
        }

        auto& queue = bufferedMessages[receiveTopic.value()];
        if (queue.empty()) return std::nullopt;

        auto msg = queue.front();
        queue.pop_front();
        logger->Info(Tag::Untagged,
            "Delivering GUID=" + msg.guid +
            " topic=" + receiveTopic.value() +
            " payload=" + msg.payload);
        return msg;
    }

    Public Bool SendMessage(CStdString& topic, const MqttMessage& msg) override {
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

#endif // ESPIDF_MQTT_CLIENT_INTERNAL_H