#ifndef LOCALSERVER_INTERNAL_H
#define LOCALSERVER_INTERNAL_H

#include <StandardDefines.h>
#include "communication/IServer.h"
#include "internal/05-server/02-interface/01-ITcpServer.h"


/* @Component */
class LocalServer final : public IServer {
    Public LocalServer() = default;
    Public Virtual ~LocalServer() override = default;

    /* @Autowired */
    Private ITcpServerPtr tcpServer;

    Public Bool Start() override {
        return tcpServer->Start();
    }

    Public Void Stop() override {
        tcpServer->Stop();
    }

    Public Bool Restart() override {
        return tcpServer->Restart();
    }

    Public Bool IsRunning() const override {
        return tcpServer->IsRunning();
    }
    
    Public IHttpRequestPtr ReceiveMessage() override {
        auto message = tcpServer->ReceiveMessage();
        if (message.empty()) {
            return nullptr;
        }
        auto request = IHttpRequest::GetRequest(message.value().guid, message.value().payload);
    }

    Public Bool SendMessage(CStdString& requestId, CStdString& message) override {
        MqttMessage mqttMessage = {
            .guid = requestId,
            .payload = message,
        };
        return tcpServer->SendMessage(mqttMessage);
    }
};

#endif // LOCALSERVER_INTERNAL_H