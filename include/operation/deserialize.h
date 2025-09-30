#ifndef DESERIALIZE_OPERATION_PATTERN_H
#define DESERIALIZE_OPERATION_PATTERN_H

#include <variant>
#include <operation/constant_pattern.h>
#include <operation/percentage_pattern.h>
#include <operation/mixed_pattern.h>

namespace Deserialize {
    using OperationPatternVariant = std::variant<
        OperationPattern::ConstantOperationPattern,
        OperationPattern::PercentageOperationPattern,
        OperationPattern::MixedOperationPattern>;
    
    inline static const std::unordered_map<
        std::string,
        std::function<OperationPatternVariant(const json& j)>>
    operation_pattern_variant_map = {
        {"constant", [](const json& j) {
            return j.template get<OperationPattern::ConstantOperationPattern>();
        }},
        {"percentage", [](const json& j) {
            return j.template get<OperationPattern::PercentageOperationPattern>();
        }},
        {"mixed", [](const json& j) {
            return j.template get<OperationPattern::MixedOperationPattern>();
        }}
    };
};

#endif
