#include <operation/constant.h>

namespace Operation {

    ConstantOperation::ConstantOperation()
        : operation(OperationType::READ) {}

    OperationType ConstantOperation::nextOperation(void) {
        return operation;
    }

    void ConstantOperation::validate(void) const {
        if (operation != OperationType::READ && operation != OperationType::WRITE) {
            throw std::invalid_argument("Invalid operation for ConstantOperationConfig");
        }
    }

    void from_json(const json& j, ConstantOperation& config) {
        if (j.at("operation").template get<std::string>() == "write") {
            config.operation = OperationType::WRITE;
        } else {
            config.operation = OperationType::READ;
        }
        config.validate();
    }
}