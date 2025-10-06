#ifndef SEQUENCE_OPERATION_H
#define SEQUENCE_OPERATION_H

#include <vector>
#include <nlohmann/json.hpp>
#include <operation/type.h>

using json = nlohmann::json;

namespace Operation {
    struct SequenceOperationConfig {
        std::vector<OperationType> operations;

        void validate(void) const {
            if (operations.size() == 0) {
                throw std::invalid_argument("Invalid operations for SequenceOperationConfig");
            }
        }
    };

    struct SequenceOperation {
        private:
            const SequenceOperationConfig config;
            size_t index;
            const size_t length;

        public:
            explicit SequenceOperation(const SequenceOperationConfig& _config)
                : config(_config), index(0), length(_config.operations.size()) {}

            OperationType nextOperation(void) {
                OperationType operation = config.operations.at(index);
                index = (index + 1) % length;
                return operation;
            }
    };

    void from_json(const json& j, SequenceOperationConfig& config) {
        for (auto& item : j.at("operations")) {
            config.operations.push_back(
                item.template get<std::string>() == "write" 
                ? OperationType::WRITE
                : OperationType::READ
            );
        }
        config.validate();
    }
};

#endif