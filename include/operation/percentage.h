#ifndef PERCENTAGE_OPERATION_H
#define PERCENTAGE_OPERATION_H

#include <nlohmann/json.hpp>
#include <operation/type.h>
#include <lib/distribution/distribution.h>

using json = nlohmann::json;

namespace Operation {
    struct PercentageOperationConfig {
        unsigned int read_percentage;

        void validate(void) const {
            if (read_percentage > 100) {
                throw std::invalid_argument("Invalid read_percentage for PercentageOperationConfig");
            }
        }
    };

    struct PercentageOperation {
        private:
            const PercentageOperationConfig config;
            Distribution::UniformDistribution<unsigned int> distribution;

        public:
            explicit PercentageOperation(const PercentageOperationConfig& _config)
                : config(_config), distribution(0, 100) {}

            OperationType nextOperation(void) {
                return (distribution.nextValue() < config.read_percentage)
                    ? OperationType::READ
                    : OperationType::WRITE;
            }
    };

    void from_json(const json& j, PercentageOperationConfig& config) {
        j.at("read_percentage").get_to(config.read_percentage);
        config.validate();
    }
};

#endif