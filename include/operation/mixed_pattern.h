#ifndef MIXED_OPERATION_PATTERN_H
#define MIXED_OPERATION_PATTERN_H

#include <nlohmann/json.hpp>
#include <operation/type.h>

using json = nlohmann::json;

namespace OperationPattern {
    struct MixedOperationPattern {
        size_t index;
        size_t length;
        std::vector<OperationType> operations;

        explicit MixedOperationPattern()
            : index(0), length(0), operations{} {}

        explicit MixedOperationPattern(const std::vector<OperationType>& _operations)
            : index(0), length(_operations.size()), operations(_operations) {}

        OperationType nextOperation() {
            const OperationType operation = operations.at(index);
            index = (index + 1) % length;
            return operation;
        }
    };

    void to_json(json& j, const MixedOperationPattern& mixed_pattern) {
        j = json{{"type", "mixed"}};
        for (OperationType operation : mixed_pattern.operations) {
            j["operations"].push_back(operation == OperationType::READ ? "read" : "write");
        }
    }

    void from_json(const json& j, MixedOperationPattern& mixed_pattern) {
        if (j.at("type").get<std::string>() != "mixed") {
            throw std::runtime_error("Invalid JSON type for MixedOperationPattern");
        }
        for (auto& item : j.at("operations")) {
            if (item == "read") {
                mixed_pattern.operations.push_back(OperationType::READ);
            } else {
                mixed_pattern.operations.push_back(OperationType::WRITE);
            }
        }
        mixed_pattern.length = mixed_pattern.operations.size();
    }
};

#endif