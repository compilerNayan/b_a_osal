#ifndef IMMQTT_CLIENT_INTERNAL_H
#define IMMQTT_CLIENT_INTERNAL_H

#include <StandardDefines.h>
#include "../01-type/01-MqttMessage.h"

DefineStandardPointers(IMqttClient)
class IMqttClient {
    Public Virtual ~IMqttClient() = default;
    
    // Lifecycle
    Public Virtual Bool Connect() = 0;
    Public Virtual Bool Disconnect() = 0;
    Public Virtual Bool IsConnected() const = 0;
    Public Virtual Bool RefreshConnection() = 0;
    Public Virtual Bool WaitForConnection(Int timeoutMs) = 0;

    // Subscribe to a topic
    Public Virtual Void Subscribe(CStdString& topic) = 0;

    // Unsubscribe from a topic
    Public Virtual Void Unsubscribe(CStdString& topic) = 0;
    Public Virtual Void UnsubscribeAll() = 0;

    // Core I/O loops (non-returning, just buffer internally)
    Public Virtual Void SendMessage() = 0;                       // checks internal send buffer, sends one if available

    // Application-facing buffer access
    Public Virtual Optional<MqttMessage> GetNextReceivedMessage(CStdString& topic) = 0; // pop from receive buffer
    Public Virtual Void QueueMessageToSend(CStdString& topic, const MqttMessage& msg) = 0; // push into send buffer

    // Optional monitoring helpers
    Public Virtual Size GetPendingReceivedCount(CStdString& topic) const = 0;
    Public Virtual Size GetPendingSendCount() const = 0;
};

#endif // IMMQTT_CLIENT_INTERNAL_H
