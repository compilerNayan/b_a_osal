#ifndef ITCPSERVER_INTERNAL_H
#define ITCPSERVER_INTERNAL_H

#include <StandardDefines.h>
#include "../01-type/01-MqttMessage.h"

DefineStandardPointers(ITcpServer)
class ITcpServer {
    Public Virtual ~ITcpServer() = default;
    
    // Lifecycle
    Public Virtual Bool Start() = 0;
    Public Virtual Bool Stop() = 0;
    Public Virtual Bool IsRunning() const = 0;
    Public Virtual Bool Restart() = 0;

    // Core I/O loops (non-returning, just buffer internally)
    Public Virtual Void ReceiveMessage() = 0;   // pulls from socket, stores in internal receive list
    Public Virtual Void SendMessage() = 0;      // checks internal send list, sends one if available

    // Application-facing buffer access
    Public Virtual Optional<MqttMessage> GetNextReceivedMessage() = 0; // pop from receive list
    Public Virtual Bool QueueMessageToSend(const MqttMessage& msg) = 0; // push into send list

    // Optional monitoring helpers
    Public Virtual Size GetPendingReceivedCount() const = 0;
    Public Virtual Size GetPendingSendCount() const = 0;
};

#endif // ITCPSERVER_INTERNAL_H
