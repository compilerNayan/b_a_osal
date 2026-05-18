#ifndef MQTTCLIENTTHREAD_H
#define MQTTCLIENTTHREAD_H

#include <StandardDefines.h>
#include "thread.h"
#include "internal/05-server/02-interface/02-IMqttClient.h"
#include "logger/ILogger.h"
#include "threading/IRunnable.h"

class MqttClientThread final : public IRunnable {
    /* @Autowired */
    Private IMqttClientPtr mqttClient_;

    /* @Autowired */
    Private ILoggerPtr logger_;

    Public MqttClientThread() = default;

    Public Virtual ~MqttClientThread() = default;

    // IRunnable implementation
    Public Virtual Void Run() override {
        logger_->Info(Tag::Untagged, "MqttClientThread started");
        mqttClient_->RefreshConnection();
        while (mqttClient_ && mqttClient_->IsConnected()) {
            mqttClient_->SendMessage();
            Thread::Sleep(400); // sleep 400 ms
        }
        logger_->Info(Tag::Untagged, "MqttClientThread stopped");
    }
};


#endif // MQTTCLIENTTHREAD_H