#ifndef FLEETPROVISIONINGPROFILEDATA_H
#define FLEETPROVISIONINGPROFILEDATA_H

#include <StandardDefines.h>

class FleetProvisioningProfileData {
    Public StdString mqttEndpoint;

    Public StdString caCertificatePem;

    Public StdString clientCertificatePem;

    Public StdString clientPrivateKeyPem;

    Public StdString createKeysRequestTopic;

    Public StdString createKeysAcceptedTopic;

    Public StdString createKeysRejectedTopic;

    Public StdString provisionRequestTopic;

    Public StdString provisionAcceptedTopic;

    Public StdString provisionRejectedTopic;
};

#endif // FLEETPROVISIONINGPROFILEDATA_H