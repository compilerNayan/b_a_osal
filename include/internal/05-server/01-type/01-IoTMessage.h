#ifndef IOTMESSAGE_INTERNAL_H
#define IOTMESSAGE_INTERNAL_H

#include <StandardDefines.h>

DefineStandardTypes(IoTMessage)
struct IoTMessage {
    StdString guid;                 // unique ID per message
    StdString payload;              // actual message content
};

#endif // IOTMESSAGE_INTERNAL_H