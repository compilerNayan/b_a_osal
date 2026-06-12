#ifndef PUBLISHTOPICS_DATA_H
#define PUBLISHTOPICS_DATA_H

#include <StandardDefines.h>

class PublishTopicsData {
    Public StdString statusTopic;
    Public StdString telemetryTopic;
    Public StdString logsTopic;
    Public StdString eventsTopic;
    Public StdString water30mBucketTopic;
    Public StdString water1sBucketTopic;
    Public StdString lifecycleEnrolledTopic;
};

#endif // PUBLISHTOPICS_DATA_H