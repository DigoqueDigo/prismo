#ifndef PERCENTAGE_OPERATION_PATTERN_H
#define PERCENTAGE_OPERATION_PATTERN_H

#include <nlohmann/json.hpp>
#include <operation/type.h>
#include <distribution/distribution.h>

using json = nlohmann::json;

namespace OperationPattern {
    struct PercentageOperationPattern {
        unsigned int read_percentage;
        Distribution::UniformDistribution<unsigned int> distribution;

        explicit PercentageOperationPattern() :
            read_percentage(0), distribution(0, 100) {}

        explicit PercentageOperationPattern(unsigned int _read_percentage)
            : read_percentage(_read_percentage), distribution(0, 100) {}

        OperationType nextOperation() {
            return (distribution.nextValue() < read_percentage)
                ? OperationType::READ
                : OperationType::WRITE;
        }
    };

    void to_json(json& j, const PercentageOperationPattern& percentage_pattern) {
        j = json{
            {"type", "percentage"},
            {"read_percentage", percentage_pattern.read_percentage},
        };
    }

    void from_json(const json& j, PercentageOperationPattern& percentage_pattern) {
        if (j.at("type").get<std::string>() != "percentage") {
            throw std::runtime_error("Invalid JSON type for PercentageOperationPattern");
        }
        j.at("read_percentage").get_to(percentage_pattern.read_percentage);
    }
};

#endif