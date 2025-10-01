#ifndef DESERIALIZE_ACCESS_PATTERN_H
#define DESERIALIZE_ACCESS_PATTERN_H

#include <variant>
#include <access/sequential_pattern.h>
#include <access/random_pattern.h>
#include <access/zipfian_pattern.h>

namespace Deserialize {
    using AccessPatternVariant = std::variant<
        AccessPattern::SequentialAccessPattern,
        AccessPattern::RandomAccessPattern,
        AccessPattern::ZipfianAccessPattern>;

    inline static const std::unordered_map<
        std::string,
        std::function<AccessPatternVariant(const json& j)>>
    access_pattern_variant_map = {
        {"sequential", [](const json& j) {
            auto config = j.template get<AccessPattern::SequentialAccessPatternConfig>();
            return AccessPattern::SequentialAccessPattern(config);
        }},
        {"random", [](const json& j) {
            auto config = j.template get<AccessPattern::RandomAccessPatternConfig>();
            return AccessPattern::RandomAccessPattern(config);
        }},
        {"zipfian", [](const json& j) {
            auto config = j.template get<AccessPattern::ZipfianAccessPatternConfig>();
            return AccessPattern::ZipfianAccessPattern(config);
        }}
    };
}

#endif