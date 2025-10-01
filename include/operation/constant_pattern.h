#ifndef CONSTANT_OPERATION_PATTERN_H
#define CONSTANT_OPERATION_PATTERN_H

#include <nlohmann/json.hpp>
#include <operation/type.h>

using json = nlohmann::json;

namespace OperationPattern {
    struct ConstantOperationPatternConfig {
        OperationType operation;

        void validate(void) {
            if (operation != OperationType::READ && operation != OperationType::WRITE) {
                throw std::invalid_argument("Invalid operation for ConstantOperationPatternConfig");
            }
        }
    };

    struct ConstantOperationPattern {
        private:
            const ConstantOperationPatternConfig config;

        public:
            explicit ConstantOperationPattern(const ConstantOperationPatternConfig& _config)
                : config(_config) {}

            OperationType nextOperation(void) {
                return config.operation;
            }
    };

    void to_json(json& j, const ConstantOperationPatternConfig& config) {
        j = json{{"type", "constant"}};
        if (config.operation == OperationType::READ) {
            j["operation"] = "read";
        } else {
            j["operation"] = "write";
        }
    }

    void from_json(const json& j, ConstantOperationPatternConfig& config) {
        if (j.at("operation").template get<std::string>() == "write") {
            config.operation = OperationType::WRITE;
        }
        config.validate();
    }
};

#endif