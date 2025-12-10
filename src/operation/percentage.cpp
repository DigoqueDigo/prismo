#include <operation/synthetic.h>

namespace Operation {

    PercentageOperation::PercentageOperation()
        : Operation(), percentages(), distribution(0, 99) {}

    OperationType PercentageOperation::nextOperation(void) {
        uint32_t roll = distribution.nextValue();
        for (auto const& [cumulative, operation] : percentages) {
            if (roll < cumulative) {
                return operation;
            }
        }
        throw std::runtime_error("Can not get nextOperation");
    }

    void from_json(const json& j, PercentageOperation& config) {
        uint32_t cumulative = 0;
        for (const auto& item: j.at("percentages").items()) {
            std::string operation = item.key();
            cumulative += item.value().template get<uint32_t>();
            config.percentages.emplace_back(cumulative, operation_from_str(operation));
        }
        if (cumulative != 100) {
            throw std::invalid_argument("Cumulative percentage different of 100 in PercentageOperation");
        }
    }
}
