#ifndef IOTMESSAGE_INTERNAL_H
#define IOTMESSAGE_INTERNAL_H

#include <StandardDefines.h>

struct IoTMessage {
    StdString guid;                 // unique ID per message
    StdString payload;              // actual message content
    Optional<StdString> address;    // sender info (IP/port, etc.)    
};

#endif // IOTMESSAGE_INTERNAL_H