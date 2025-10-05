#ifndef PERCENTAGE_OPERATION_PATTERN_H
#define PERCENTAGE_OPERATION_PATTERN_H

#include <nlohmann/json.hpp>
#include <operation/type.h>
#include <lib/distribution/distribution.h>

using json = nlohmann::json;

namespace OperationPattern {
    struct PercentageOperationPatternConfig {
        unsigned int read_percentage;

        void validate(void) const {
            if (read_percentage > 100) {
                throw std::invalid_argument("Invalid read_percentage for PercentageOperationPatternConfig");
            }
        }
    };

    struct PercentageOperationPattern {
        private:
            const PercentageOperationPatternConfig config;
            Distribution::UniformDistribution<unsigned int> distribution;

        public:
            explicit PercentageOperationPattern(const PercentageOperationPatternConfig& _config)
                : config(_config), distribution(0, 100) {}

            OperationType nextOperation(void) {
                return (distribution.nextValue() < config.read_percentage)
                    ? OperationType::READ
                    : OperationType::WRITE;
            }
    };

    void from_json(const json& j, PercentageOperationPatternConfig& config) {
        j.at("read_percentage").get_to(config.read_percentage);
        config.validate();
    }
};

#endif