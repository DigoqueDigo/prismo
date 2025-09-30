#ifndef ACCESS_PATTERN_DESERIALIZE_H
#define ACCESS_PATTERN_DESERIALIZE_H

#include <variant>
#include <operation/constant_pattern.h>
#include <operation/percentage_pattern.h>
#include <operation/mixed_pattern.h>

namespace Deserialize {
    using PatternVariant = std::variant<
        OperationPattern::ConstantOperationPattern,
        OperationPattern::PercentageOperationPattern,
        OperationPattern::MixedOperationPattern>;
    
    inline static const std::unordered_map<std::string, std::function<PatternVariant(const json& j)>> pattern_variant_map = {
        {"constant", [](const json& j) {
            return j.template get<OperationPattern::ConstantOperationPattern>();
        }},
        {"percentage", [](const json& j){
            return j.template get<OperationPattern::PercentageOperationPattern>();
        }},
        {"mixed", [](const json& j){
            return j.template get<OperationPattern::MixedOperationPattern>();
        }}
    };
};

#endif
