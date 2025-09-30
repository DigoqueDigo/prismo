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
        j = json{{"type", "constant"}};
        if (constant_pattern.operation == OperationType::READ) {
            j["operation"] = "read";
        } else {
            j["operation"] = "write";
        }
    }

    void from_json(const json& j, ConstantOperationPattern& constant_pattern) {
        if (j.at("type") != "constant") {
            throw std::runtime_error("Invalid JSON type for ConstantOperationPattern");
        }
        if (j.at("operation") == "read") {
            constant_pattern.operation = OperationType::READ;
        } else if (j.at("operation") == "write") {
            constant_pattern.operation = OperationType::WRITE;
        } else {
            throw std::runtime_error("Invalid JSON operation for ConstantOperationPattern");
        }
    }
};

#endif