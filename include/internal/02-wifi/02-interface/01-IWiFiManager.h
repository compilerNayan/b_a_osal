#ifndef IWIFI_MANAGER_INTERNAL_H
#define IWIFI_MANAGER_INTERNAL_H

#include <StandardDefines.h>
#include "../01-type/01-WiFiConnectionStatus.h"

DefineStandardPointers(IWiFiManager)
class IWiFiManager {
    Public Virtual ~IWiFiManager() = default;

    Public Virtual Bool Connect(CStdString& ssid, const Optional<CStdString> password) = 0;
    Public Virtual Void Disconnect() = 0;
    Public Virtual Bool IsConnected() const = 0;
    Public Virtual Bool WaitForConnection(Int timeoutMs) = 0;
    Public Virtual WiFiConnectionStatus GetStatus() const = 0;
    Public Virtual Optional<StdString> GetIPAddress() const = 0;
    Public Virtual StdString GetMACAddress() const = 0;
    Public Virtual StdVector<StdString> ScanNetworks() const = 0;
};

#endif // IWIFI_MANAGER_INTERNAL_H