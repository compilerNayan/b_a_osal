#ifndef MQTT_MESSAGE_INTERNAL_H
#define MQTT_MESSAGE_INTERNAL_H

#include <StandardDefines.h>

struct MqttMessage {
    StdString guid;
    StdString payload;
};

#endif // MQTT_MESSAGE_INTERNAL_H
