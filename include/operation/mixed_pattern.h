#ifndef MIXED_OPERATION_PATTERN_H
#define MIXED_OPERATION_PATTERN_H

#include <nlohmann/json.hpp>
#include <operation/type.h>

using json = nlohmann::json;

namespace OperationPattern {
    struct MixedOperationPattern {
        size_t index;
        size_t length;
        std::vector<OperationType> pattern;

        explicit MixedOperationPattern()
            : index(0), length(0), pattern{} {}

        // explicit MixedOperationPattern(const std::vector<OperationType>& _pattern)
        //     : index(0), length(_pattern.size()), pattern(_pattern) {}

        OperationType nextOperation() {
            const OperationType operation = pattern.at(index);
            index = (index + 1) % length;
            return operation;
        }
    };

    void to_json(json& j, const MixedOperationPattern& mixed_pattern) {
        j = json{{"type", "mixed"}};
        for (OperationType operation : mixed_pattern.pattern) {
            j["pattern"].push_back(operation == OperationType::READ ? "read" : "write");
        }
    }

    void from_json(const json& j, MixedOperationPattern& mixed_pattern) {
        if (j.at("type").template get<std::string>() != "mixed") {
            throw std::runtime_error("Invalid JSON type for MixedOperationPattern");
        }
        for (auto& item : j.at("pattern")) {
            if (item.template get<std::string>() == "read") {
                mixed_pattern.pattern.push_back(OperationType::READ);
            } else if (item.template get<std::string>() == "write") {
                mixed_pattern.pattern.push_back(OperationType::WRITE);
            } else {
                throw std::runtime_error("Invalid JSON pattern for MixedOperationPattern");
            }
        }
        mixed_pattern.length = mixed_pattern.pattern.size();
    }
};

#endif