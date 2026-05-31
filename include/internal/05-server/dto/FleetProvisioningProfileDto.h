#ifndef FLEETPROVISIONINGPROFILEDTO_H
#define FLEETPROVISIONINGPROFILEDTO_H

#include <StandardDefines.h>

/* @Serializable */
class FleetProvisioningProfileDto {

    Public optional<StdString> mqttEndpoint;

    Public optional<StdString> caCertificatePem;
    Public optional<StdString> clientCertificatePem;
    Public optional<StdString> clientPrivateKeyPem;

    Public optional<StdString> templateName;

    Public optional<StdString> createKeysRequestTopic;
    Public optional<StdString> createKeysAcceptedTopic;
    Public optional<StdString> createKeysRejectedTopic;

    Public optional<StdString> provisionRequestTopicPrefix;
    Public optional<StdString> provisionRequestTopicSuffix;
    Public optional<StdString> provisionRequestTopic;

    Public optional<StdString> provisionAcceptedTopicPrefix;
    Public optional<StdString> provisionAcceptedTopicSuffix;
    Public optional<StdString> provisionAcceptedTopic;

    Public optional<StdString> provisionRejectedTopicPrefix;
    Public optional<StdString> provisionRejectedTopicSuffix;
    Public optional<StdString> provisionRejectedTopic;
};

#endif // FLEETPROVISIONINGPROFILEDTO_H