#ifndef PUBLISHTOPICS_H
#define PUBLISHTOPICS_H

#include <StandardDefines.h>

/*--@Serializable--*/
class PublishTopics {

    Public optional<StdString> statusTopic;

    Public optional<StdString> telemetryTopic;

    Public optional<StdString> logsTopic;

    Public optional<StdString> eventsTopic;

        // Serialization method
        Public StdString Serialize() const {
            // Create JSON document
            JsonDocument doc;

            // Serialize optional field: statusTopic
            if (statusTopic.has_value()) {
                doc["statusTopic"] = statusTopic.value().c_str();
            } else {
                doc["statusTopic"] = nullptr;
            }
            // Serialize optional field: telemetryTopic
            if (telemetryTopic.has_value()) {
                doc["telemetryTopic"] = telemetryTopic.value().c_str();
            } else {
                doc["telemetryTopic"] = nullptr;
            }
            // Serialize optional field: logsTopic
            if (logsTopic.has_value()) {
                doc["logsTopic"] = logsTopic.value().c_str();
            } else {
                doc["logsTopic"] = nullptr;
            }
            // Serialize optional field: eventsTopic
            if (eventsTopic.has_value()) {
                doc["eventsTopic"] = eventsTopic.value().c_str();
            } else {
                doc["eventsTopic"] = nullptr;
            }

            // Serialize to string
            StdString output;
            serializeJson(doc, output);

            return StdString(output.c_str());
        }

            // Validation method for all validation macros
            #pragma GCC diagnostic push
            #pragma GCC diagnostic ignored "-Wunused-parameter"
            Public template<typename DocType>
            Static StdString ValidateFields(DocType& doc) {
            StdString validationErrors;

            // No validation macros defined for this class

            return validationErrors;
        }
            #pragma GCC diagnostic pop

        // Deserialization method
        Public Static PublishTopics Deserialize(const StdString& input) {
            // Create JSON document
            JsonDocument doc;

            // Deserialize JSON string
            DeserializationError error = deserializeJson(doc, input.c_str());

            if (error) {
                StdString errorMsg = "JSON parse error: ";
                errorMsg += error.c_str();
                throw std::runtime_error(errorMsg.c_str());
            }

            // Validate all fields with validation macros
            StdString validationErrors = ValidateFields(doc);
            if (!validationErrors.empty()) {
                throw std::runtime_error(validationErrors.c_str());
            }

            // Create object with default constructor
            PublishTopics obj;

            // Assign values from JSON if present (only optional fields)
            // Deserialize optional field: statusTopic
            if (!doc["statusTopic"].isNull()) {
                obj.statusTopic = StdString(doc["statusTopic"].as<const char*>());
            }
            // Deserialize optional field: telemetryTopic
            if (!doc["telemetryTopic"].isNull()) {
                obj.telemetryTopic = StdString(doc["telemetryTopic"].as<const char*>());
            }
            // Deserialize optional field: logsTopic
            if (!doc["logsTopic"].isNull()) {
                obj.logsTopic = StdString(doc["logsTopic"].as<const char*>());
            }
            // Deserialize optional field: eventsTopic
            if (!doc["eventsTopic"].isNull()) {
                obj.eventsTopic = StdString(doc["eventsTopic"].as<const char*>());
            }

            return obj;
        }
};

#endif // PUBLISHTOPICS_H