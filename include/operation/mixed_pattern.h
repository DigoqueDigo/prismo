#ifndef MIXED_OPERATION_PATTERN_H
#define MIXED_OPERATION_PATTERN_H

#include <nlohmann/json.hpp>
#include <operation/type.h>

using json = nlohmann::json;

namespace OperationPattern {
    struct MixedOperationPatternConfig {
        std::vector<OperationType> pattern;   

        void validate(void) {
            if (pattern.size() == 0) {
                throw std::invalid_argument("Invalid pattern for MixedOperationPatternConfig");
            }
        }
    };

    struct MixedOperationPattern {
        private:
            const MixedOperationPatternConfig config;
            size_t index;
            const size_t length;

        public:
            explicit MixedOperationPattern(const MixedOperationPatternConfig& _config)
                : config(_config), index(0), length(_config.pattern.size()) {}

            OperationType nextOperation(void) {
                const OperationType operation = config.pattern.at(index);
                index = (index + 1) % length;
                return operation;
            }
    };

    void to_json(json& j, const MixedOperationPatternConfig& config) {
        j = json{{"type", "mixed"}};
        for (OperationType operation : config.pattern) {
            j["pattern"].push_back(operation == OperationType::READ ? "read" : "write");
        }
    }

    void from_json(const json& j, MixedOperationPatternConfig& config) {
        for (auto& item : j.at("pattern")) {
            config.pattern.push_back(
                item.template get<std::string>() == "write" 
                ? OperationType::WRITE
                : OperationType::READ
            );
        }
        config.validate();
    }
};

#endif