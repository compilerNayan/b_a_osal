#ifndef IOTMESSAGE_INTERNAL_H
#define IOTMESSAGE_INTERNAL_H

#include <StandardDefines.h>

struct IoTMessage {
    StdString guid;
    StdString payload;
    Optional<StdString> address;  // MQTT topic or transport path
};

#endif // IOTMESSAGE_INTERNAL_H
