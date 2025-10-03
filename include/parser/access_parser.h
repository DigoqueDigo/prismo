#ifndef ACCESS_PATTERN_PARSER_H
#define ACCESS_PATTERN_PARSER_H

#include <variant>
#include <stdexcept>
#include <functional>
#include <unordered_map>
#include <access/sequential_pattern.h>
#include <access/random_pattern.h>
#include <access/zipfian_pattern.h>

namespace Parser {
    using AccessPatternVariant = std::variant<
        AccessPattern::SequentialAccessPattern,
        AccessPattern::RandomAccessPattern,
        AccessPattern::ZipfianAccessPattern>;

    inline static const std::unordered_map<
        std::string,
        std::function<AccessPatternVariant(json& specialized, const json& workload)>>
    access_pattern_variant_map = {
        {"sequential", [](json& specialized, const json& workload) {
            specialized.merge_patch(workload);
            auto config = specialized.template get<AccessPattern::SequentialAccessPatternConfig>();
            return AccessPattern::SequentialAccessPattern(config);
        }},
        {"random", [](json& specialized, const json& workload) {
            specialized.merge_patch(workload);
            auto config = specialized.template get<AccessPattern::RandomAccessPatternConfig>();
            return AccessPattern::RandomAccessPattern(config);
        }},
        {"zipfian", [](json& specialized, const json& workload) {
            specialized.merge_patch(workload);
            auto config = specialized.template get<AccessPattern::ZipfianAccessPatternConfig>();
            return AccessPattern::ZipfianAccessPattern(config);
        }}
    };

    AccessPatternVariant getAccessPattern(const std::string& type, json& specialized, const json& workload) {
        auto it  = access_pattern_variant_map.find(type);
        if (it != access_pattern_variant_map.end()) {
            return it->second(specialized, workload);
        } else {
            throw std::invalid_argument("Access pattern type '" + type + "' is not recognized");
        }
    }
}

#endif