#ifndef PERCENTAGE_OPERATION_H
#define PERCENTAGE_OPERATION_H

#include <vector>
#include <utility>
#include <operation/type.h>
#include <nlohmann/json.hpp>
#include <lib/distribution/distribution.h>

using json = nlohmann::json;

namespace Operation {

    struct PercentageOperation {
        private:
            std::vector<std::pair<OperationType, int>> percentages;
            Distribution::UniformDistribution<uint32_t> distribution;

        public:
            PercentageOperation();
            OperationType nextOperation(void);
            void validate(void) const;
            friend void from_json(const json& j, PercentageOperation& config);
    };

    void from_json(const json& j, PercentageOperation& config);
};

#endif