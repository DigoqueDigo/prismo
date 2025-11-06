#include <operation/synthetic.h>

namespace Operation {

    ConstantOperation::ConstantOperation()
        : Operation(), operation(OperationType::READ) {}

    OperationType ConstantOperation::nextOperation(void) {
        return operation;
    }

    void from_json(const json& j, ConstantOperation& config) {
        std::string operation = j.at("operation").template get<std::string>();
        config.operation = operation_from_str(operation);
    }
}