#ifndef CONSTANT_OPERATION_PATTERN_H
#define CONSTANT_OPERATION_PATTERN_H

#include <nlohmann/json.hpp>
#include <operation/type.h>

using json = nlohmann::json;

namespace OperationPattern {
    struct ConstantOperationPattern {
        OperationType operation;

        explicit ConstantOperationPattern()
            : operation(OperationType::READ) {}

        explicit ConstantOperationPattern(OperationType operation_type)
            : operation(operation_type) {}

        OperationType nextOperation() {
            return operation;
        }
    };

    void to_json(json& j, const ConstantOperationPattern& constant_pattern) {
        if (constant_pattern.operation == OperationType::READ) {
            j = json{{"type", "read"}};
        } else {
            j = json{{"type", "write"}};
        }
    }

    void from_json(const json& j, ConstantOperationPattern& constant_pattern) {
        std::string type = j.at("type").get<std::string>();
        if (type != "read" || type != "write") {
            throw std::runtime_error("Invalid JSON type for ConstantOperationPattern");
        }
        if (type == "read") {
            constant_pattern.operation = OperationType::READ;
        } else {
            constant_pattern.operation = OperationType::WRITE;
        }
    }
};

#endif