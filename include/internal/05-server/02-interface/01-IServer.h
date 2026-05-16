#ifndef ISERVER_INTERNAL_H
#define ISERVER_INTERNAL_H

#include <StandardDefines.h>
#include "../01-type/01-IoTMessage.h"

DefineStandardPointers(IServer)
class IServer {
    Public Virtual ~IServer() = default;
    
    Public Virtual Bool Start() = 0;
    Public Virtual Bool Stop() = 0;
    Public Virtual Bool IsRunning() const = 0;
    Public Virtual Bool Restart() = 0;
    
    Public Virtual Optional<IoTMessage> ReceiveMessage(Optional<StdString> path = std::nullopt) = 0;
    Public Virtual Bool SendMessage(const IoTMessage& msg, Optional<StdString> path = std::nullopt) = 0;
};

#endif // ISERVER_INTERNAL_H