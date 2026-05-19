#ifndef CLOUDSERVER_INTERNAL_H
#define CLOUDSERVER_INTERNAL_H

#include <StandardDefines.h>
#include "communication/ICloudServer.h"
#include "internal/05-server/02-interface/02-IMqttClient.h"

/* @Component */
class CloudServer final : public ICloudServer {
    Public CloudServer() = default;
    Public Virtual ~CloudServer() override = default;

    /* @Autowired */
    Private IMqttClientPtr mqttClient;

    Public Bool Start() override {
        mqttClient->Connect();
        if(mqttClient->WaitForConnection(10000)) {
            mqttClient->Subscribe("nknk32/sub");
            return true;
        }
        return false;
    }

    Public Void Stop() override {
        mqttClient->Disconnect();
    }

    Public Bool Restart() override {
        mqttClient->Subscribe("nknk32/sub");
        return mqttClient->RefreshConnection();
    }
    
    Public Bool IsRunning() const override {
        return mqttClient->IsConnected();
    }
    
    Public IHttpRequestPtr ReceiveMessage() override {
        CStdString path = "nknk32/sub";
        auto message = mqttClient->GetNextReceivedMessage(path);
        if (!message.has_value()) {
            return nullptr;
        }
        auto request = IHttpRequest::GetRequest(message.value().guid, RequestSource::CloudServer, message.value().payload);
        return request;
    }

    Public Bool SendMessage(CStdString& requestId, CStdString& message) override {
        CStdString path = "nknk32/pub";
        MqttMessage mqttMessage = {
            .guid = requestId,
            .payload = message,
        };
        return mqttClient->QueueMessageToSend(path, mqttMessage);
    }
};
#endif // CLOUDSERVER_INTERNAL_H