#ifndef ITCPSERVER_INTERNAL_H
#define ITCPSERVER_INTERNAL_H

#include <StandardDefines.h>
#include "../01-type/01-MqttMessage.h"

DefineStandardPointers(ITcpServer)
class ITcpServer {
    Public Virtual ~ITcpServer() = default;
    
    Public Virtual Bool Start() = 0;
    Public Virtual Bool Stop() = 0;
    Public Virtual Bool IsRunning() const = 0;
    Public Virtual Bool Restart() = 0;
    
    Public Virtual Optional<MqttMessage> ReceiveMessage() = 0;
    Public Virtual Bool SendMessage(const MqttMessage& msg) = 0;
};

#endif // ITCPSERVER_INTERNAL_H