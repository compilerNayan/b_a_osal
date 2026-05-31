#ifndef DEVICEIDENTITYPROFILEDATA_H
#define DEVICEIDENTITYPROFILEDATA_H

#include <StandardDefines.h>
#include "type/PublishTopicsData.h"
#include "type/SubscribeTopicsData.h"

class DeviceIdentityProfileData {
    Public StdString mqttEndpoint;
    Public StdString caCertificatePem;
    Public StdString clientCertificatePem;
    Public StdString clientPrivateKeyPem;
    Public StdString thingName;
    Public PublishTopicsData publishTopics;
    Public SubscribeTopicsData subscribeTopics;
};

#endif // DEVICEIDENTITYPROFILEDATA_H