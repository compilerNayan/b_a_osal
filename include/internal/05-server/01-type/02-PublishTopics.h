#ifndef PUBLISHTOPICS_H
#define PUBLISHTOPICS_H

#include <StandardDefines.h>

/* @Serializable */
class PublishTopics {

    Public optional<StdString> statusTopic;

    Public optional<StdString> telemetryTopic;

    Public optional<StdString> logsTopic;

    Public optional<StdString> eventsTopic;

    Public optional<StdString> water30mBucketTopic;

    Public optional<StdString> water1sBucketTopic;

    Public optional<StdString> lifecycleEnrolledTopic;

};

#endif // PUBLISHTOPICS_H