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

    void from_json(const json& j, PercentageOperation& config) {
        int cumulative = 0;
        for (const auto& item: j.at("percentages").items()) {
            std::string operation = item.key();
            int value = item.value().template get<int>();

            if (value < 0) {
                throw std::invalid_argument("Invalid percentage for operation '" + operation + "'");
            } else {
                cumulative += value;
                config.percentages.emplace_back(operation_from_str(operation), cumulative);
            }
        }
        if (cumulative != 100) {
            throw std::invalid_argument("Cumulative percentage different of 100");
        }
    }
}
