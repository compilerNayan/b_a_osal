#ifndef IHOTSPOT_MANAGER_INTERNAL_H
#define IHOTSPOT_MANAGER_INTERNAL_H

#include <StandardDefines.h>

DefineStandardPointers(IHotspotManager)
class IHotspotManager {
    Public Virtual ~IHotspotManager() = default;
    
    Public Virtual Bool Start(CStdString& ssid, Optional<CStdString>& password, Int maxClients = 4) = 0;
    Public Virtual Void Stop() = 0;
    Public Virtual Bool IsActive() const = 0;
    Public Virtual optional<StdString> GetIPAddress() const = 0;
    Public Virtual Int GetConnectedClients() const = 0;
    Public Virtual StdVector<StdString> ListClients() const = 0;
    Public Virtual Bool ChangeCredentials(CStdString& ssid, CStdString& password) = 0;
};

#endif // IHOTSPOT_MANAGER_INTERNAL_H