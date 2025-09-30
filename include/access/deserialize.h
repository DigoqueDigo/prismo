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
            return j.template get<AccessPattern::SequentialAccessPattern>();
        }},
        {"random", [](const json& j) {
            return j.template get<AccessPattern::RandomAccessPattern>();
        }},
        {"zipfian", [](const json& j) {
            return j.template get<AccessPattern::ZipfianAccessPattern>();
        }}
    };
}

#endif