#ifndef CONSTANT_OPERATION_H
#define CONSTANT_OPERATION_H

#include <nlohmann/json.hpp>
#include <operation/type.h>

using json = nlohmann::json;

namespace Operation {
    struct ConstantOperationConfig {
        OperationType operation;

        void validate(void) const {
            if (operation != OperationType::READ && operation != OperationType::WRITE) {
                throw std::invalid_argument("Invalid operation for ConstantOperationConfig");
            }
        }
    };

    struct ConstantOperation {
        private:
            const ConstantOperationConfig config;

        public:
            explicit ConstantOperation(const ConstantOperationConfig& _config)
                : config(_config) {}

            OperationType nextOperation(void) {
                return config.operation;
            }
    };

    void from_json(const json& j, ConstantOperationConfig& config) {
        if (j.at("operation").template get<std::string>() == "write") {
            config.operation = OperationType::WRITE;
        }
        config.validate();
    }
};

#endif