#ifndef OPERATION_PATTERN_PARSER_H
#define OPERATION_PATTERN_PARSER_H

#include <variant>
#include <stdexcept>
#include <functional>
#include <unordered_map>
#include <operation/constant_pattern.h>
#include <operation/percentage_pattern.h>
#include <operation/mixed_pattern.h>

namespace Parser {
    using OperationPatternVariant = std::variant<
        OperationPattern::ConstantOperationPattern,
        OperationPattern::PercentageOperationPattern,
        OperationPattern::MixedOperationPattern>;

    inline static const std::unordered_map<
        std::string,
        std::function<OperationPatternVariant(const json& j)>>
    operation_pattern_variant_map = {
        {"constant", [](const json& j) {
            auto config = j.template get<OperationPattern::ConstantOperationPatternConfig>();
            return OperationPattern::ConstantOperationPattern(config);
        }},
        {"percentage", [](const json& j) {
            auto config = j.template get<OperationPattern::PercentageOperationPatternConfig>();
            return OperationPattern::PercentageOperationPattern(config);
        }},
        {"mixed", [](const json& j) {
            auto config = j.template get<OperationPattern::MixedOperationPatternConfig>();
            return OperationPattern::MixedOperationPattern(config);
        }}
    };

    OperationPatternVariant getOperationPattern(const std::string& type, const json& j) {
        auto it = operation_pattern_variant_map.find(type);
        if (it != operation_pattern_variant_map.end()) {
            return it->second(j);
        } else {
            throw std::invalid_argument("Operation pattern type '" + type + "' is not recognized");
        }
    }
};

#endif
