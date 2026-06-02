#ifndef IDEVICEIDENTITYPROVIDER_INTERNAL_H
#define IDEVICEIDENTITYPROVIDER_INTERNAL_H

#include "esp_system.h"
#include "StandardDefines.h"
#include <string>

DefineStandardPointers(IDeviceIdentityProvider)
class IDeviceIdentityProvider {
    Public Virtual ~IDeviceIdentityProvider() = default;

    Public Virtual StdString GetSerialNumber() const = 0;
    Public Virtual StdString GetDeviceSecret() const = 0;
    Public Virtual StdString GetFirmwareVersion() const = 0;
    Public Virtual StdString GetDeviceType() const = 0;
};

#endif // IDEVICEIDENTITYPROVIDER_INTERNAL_H

