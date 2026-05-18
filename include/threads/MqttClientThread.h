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
        while(!mqttClient_->IsConnected()) {
            mqttClient_->RefreshConnection();
            if(mqttClient_->WaitForConnection(10000)) {
                logger_->Info(Tag::Untagged, "MqttClientThread started");
                mqttClient_->Subscribe("nknk32/sub");
                while (mqttClient_ && mqttClient_->IsConnected()) {
                    mqttClient_->SendMessage();
                    mqttClient_->ReceiveMessage();
                    Thread::Sleep(100); // sleep 100 ms
                }
            } else {
                logger_->Error(Tag::Untagged, "MqttClientThread failed to connect");
            }
            Thread::Sleep(2000);
        }
    }
};


#endif // MQTTCLIENTTHREAD_H