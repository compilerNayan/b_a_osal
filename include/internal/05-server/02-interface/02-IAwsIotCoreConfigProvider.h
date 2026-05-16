#ifndef IAWS_IOT_CORE_CONFIG_PROVIDER_INTERNAL_H
#define IAWS_IOT_CORE_CONFIG_PROVIDER_INTERNAL_H

#include <StandardDefines.h>

/** Provides AWS IoT Core connection config as StdString values. */
DefineStandardPointers(IAwsIotCoreConfigProvider)
class IAwsIotCoreConfigProvider {
    Public Virtual ~IAwsIotCoreConfigProvider() = default;

    Public Virtual StdString GetEndpoint() const = 0;
    Public Virtual StdString GetThingName() const = 0;
    Public Virtual StdString GetCaCert() const = 0;
    Public Virtual StdString GetDeviceCert() const = 0;
    Public Virtual StdString GetPrivateKey() const = 0;
    Public Virtual StdString GetSendTopic() const = 0;
    Public Virtual StdString GetReceiveTopic() const = 0;
};

#endif /* IAWS_IOT_CORE_CONFIG_PROVIDER_INTERNAL_H */
