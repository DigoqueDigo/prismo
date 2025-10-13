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
            config.operations.push_back(
                item.template get<std::string>() == "write" 
                ? OperationType::WRITE
                : OperationType::READ
            );
        }
        config.length = config.operations.size();
        config.validate();
    }
}