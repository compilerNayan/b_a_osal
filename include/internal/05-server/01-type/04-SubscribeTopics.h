#ifndef SUBSCRIBETOPICS_H
#define SUBSCRIBETOPICS_H

#include <StandardDefines.h>

/*--@Serializable--*/
class SubscribeTopics {

    Public optional<StdString> commandTopic;

    Public optional<StdString> otaUpdateTopic;

    Public optional<StdString> featureFlagTopic;

        // Serialization method
        Public StdString Serialize() const {
            // Create JSON document
            JsonDocument doc;

            // Serialize optional field: commandTopic
            if (commandTopic.has_value()) {
                doc["commandTopic"] = commandTopic.value().c_str();
            } else {
                doc["commandTopic"] = nullptr;
            }
            // Serialize optional field: otaUpdateTopic
            if (otaUpdateTopic.has_value()) {
                doc["otaUpdateTopic"] = otaUpdateTopic.value().c_str();
            } else {
                doc["otaUpdateTopic"] = nullptr;
            }
            // Serialize optional field: featureFlagTopic
            if (featureFlagTopic.has_value()) {
                doc["featureFlagTopic"] = featureFlagTopic.value().c_str();
            } else {
                doc["featureFlagTopic"] = nullptr;
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
        Public Static SubscribeTopics Deserialize(const StdString& input) {
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
            SubscribeTopics obj;

            // Assign values from JSON if present (only optional fields)
            // Deserialize optional field: commandTopic
            if (!doc["commandTopic"].isNull()) {
                obj.commandTopic = StdString(doc["commandTopic"].as<const char*>());
            }
            // Deserialize optional field: otaUpdateTopic
            if (!doc["otaUpdateTopic"].isNull()) {
                obj.otaUpdateTopic = StdString(doc["otaUpdateTopic"].as<const char*>());
            }
            // Deserialize optional field: featureFlagTopic
            if (!doc["featureFlagTopic"].isNull()) {
                obj.featureFlagTopic = StdString(doc["featureFlagTopic"].as<const char*>());
            }

            return obj;
        }
};

#endif // SUBSCRIBETOPICS_H