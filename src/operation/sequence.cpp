#include <operation/sequence.h>

namespace Operation {

    SequenceOperation::SequenceOperation()
        : index(0), length(0), operations() {}

    OperationType SequenceOperation::nextOperation(void) {
        OperationType operation = operations.at(index);
        index = (index + 1) % length;
        return operation;
    }

    void SequenceOperation::validate(void) const {
        if (operations.size() == 0) {
            throw std::invalid_argument("Invalid operations for SequenceOperationConfig");
        }
    }

    void from_json(const json& j, SequenceOperation& config) {
        for (auto& item : j.at("operations")) {
            std::string operation = item.template get<std::string>();
            config.operations.push_back(operation_from_str(operation));
        }
        config.length = config.operations.size();
        config.validate();
    }
}