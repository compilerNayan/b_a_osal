#ifndef IMMQTT_CLIENT_INTERNAL_H
#define IMMQTT_CLIENT_INTERNAL_H

#include <StandardDefines.h>
#include "../01-type/01-MqttMessage.h"

DefineStandardPointers(IMMqttClient)
class IMMqttClient {
    Public Virtual ~IMMqttClient() = default;
    
    Public Virtual Bool Connect() = 0;
    Public Virtual Bool Disconnect() = 0;
    Public Virtual Bool IsConnected() const = 0;
    Public Virtual Bool RefreshConnection() const = 0;
    Public Virtual Bool WaitForConnection(Int timeoutMs) = 0;

    Public Virtual Optional<MqttMessage> ReceiveMessage(CStdString& topic) = 0;
    Public Virtual Bool SendMessage(CStdString& topic, const MqttMessage& msg) = 0;
};

#endif // IMMQTT_CLIENT_INTERNAL_H