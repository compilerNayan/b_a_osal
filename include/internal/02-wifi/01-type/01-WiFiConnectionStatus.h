#ifndef WIFI_CONNECTION_STATUS_INTERNAL_H
#define WIFI_CONNECTION_STATUS_INTERNAL_H

#include <StandardDefines.h>

DefineStandardTypes(WiFiConnectionStatus)
enum class WiFiConnectionStatus {
    Disconnected,
    Connected,
    Connecting,
    Disconnecting,
    Failed,
    Timeout,
    Unknown
};

#endif // WIFI_CONNECTION_STATUS_INTERNAL_H