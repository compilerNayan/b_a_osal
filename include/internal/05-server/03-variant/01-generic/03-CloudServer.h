#ifndef CLOUDSERVER_INTERNAL_H
#define CLOUDSERVER_INTERNAL_H

#include <StandardDefines.h>
#include "communication/ICloudServer.h"
#include "internal/05-server/02-interface/02-MqttServer.h"


/* @Component */
class CloudServer final : public ICloudServer {
    Public CloudServer() = default;
    Public Virtual ~CloudServer() override = default;

    /* @Autowired */
    Private IMqttServerPtr mqttServer;

    Public Bool Start() override {
        return mqttServer->Connect();
    }

    Public Void Stop() override {
        mqttServer->Disconnect();
    }

    Public Bool Restart() override {
        return mqttServer->RefreshConnection();
    }
    
    Public Bool IsRunning() const override {
        return mqttServer->IsConnected();
    }
    
    Public IHttpRequestPtr ReceiveMessage() override {
        CStdString path = "/nknk32/sub";
        auto message = mqttServer->ReceiveMessage(path);
        if (message.empty()) {
            return nullptr;
        }
        auto request = IHttpRequest::GetRequest(message.value().guid, message.value().payload);
        return request;
    }

    Public Bool SendMessage(CStdString& requestId, CStdString& message) override {
        CStdString path = "/nknk32/pub";
        MqttMessage mqttMessage = {
            .guid = requestId,
            .payload = message,
        };
        return mqttServer->SendMessage(path, mqttMessage);
    }
};
#endif // CLOUDSERVER_INTERNAL_H