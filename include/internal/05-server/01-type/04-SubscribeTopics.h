#ifndef SUBSCRIBETOPICS_H
#define SUBSCRIBETOPICS_H

#include <StandardDefines.h>

/* @Serializable */
class SubscribeTopics {

    Public optional<StdString> commandTopic;

    Public optional<StdString> otaUpdateTopic;

    Public optional<StdString> featureFlagTopic;

};

#endif // SUBSCRIBETOPICS_H