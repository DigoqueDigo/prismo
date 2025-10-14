#include <operation/percentage.h>

namespace Operation {

    PercentageOperation::PercentageOperation()
        : percentages(), distribution(0, 99) {}

    OperationType PercentageOperation::nextOperation(void) {
        int roll = distribution.nextValue();
        for (auto const& iter : percentages) {
            if (roll < iter.second) {
                return iter.first;
            }
        }
        throw std::runtime_error("Can not get nextOperation");
    }

    void PercentageOperation::validate(void) const {
        for (auto const& iter : percentages) {
            if (iter.second < 0 || iter.second > 100) {
                throw std::invalid_argument("Invalid percentage for operation");
            }
        }
    }

    void from_json(const json& j, PercentageOperation& config) {
        int cumulative = 0;
        for (const auto& item: j.at("percentages").items()) {
            std::string operation = item.key();
            cumulative += item.value().template get<int>();
            config.percentages.emplace_back(operation_from_str(operation), cumulative);
        }
        if (cumulative != 100) {
            throw std::invalid_argument("Cumulative percentage diferent of 100");
        }
        config.validate();
    }
}
