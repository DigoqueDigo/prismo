#include <operation/percentage.h>

namespace Operation {

    PercentageOperation::PercentageOperation()
        : read_percentage(0), distribution(0, 100) {}

    OperationType PercentageOperation::nextOperation(void) {
        return (distribution.nextValue() < read_percentage)
            ? OperationType::READ
            : OperationType::WRITE;
    }
    
    void PercentageOperation::validate(void) const {
        if (read_percentage > 100) {
            throw std::invalid_argument("Invalid read_percentage for PercentageOperationConfig");
        }
    }

    void from_json(const json& j, PercentageOperation& config) {
        j.at("read_percentage").get_to(config.read_percentage);
        config.validate();
    }
}
