#ifndef ZIPFIAN_ACCESS_PATTERN_H
#define ZIPFIAN_ACCESS_PATTERN_H

#include <cstddef>
#include <stdexcept>
#include <nlohmann/json.hpp>
#include <lib/distribution/distribution.h>

using json = nlohmann::json;

namespace AccessPattern {
    struct ZipfianAccessPatternConfig {
        size_t block_size;
        size_t limit;
        float skew;

        void validate(void) {
            if (block_size == 0)
                throw std::invalid_argument("Invalid block_size for ZipfianAccessPatternConfig");
            if (block_size > limit)
                throw std::invalid_argument("Invalid limit for ZipfianAccessPatternConfig");
            if (skew <= 0 || skew >= 1)
                throw std::invalid_argument("Invalid skew for ZipfianAccessPatternConfig");
        }
    };

    struct ZipfianAccessPattern {
        private:
            const ZipfianAccessPatternConfig config;
            Distribution::ZipfianDistribution<size_t> distribution;

        public:
            explicit ZipfianAccessPattern(const ZipfianAccessPatternConfig& _config)
                : config(_config), distribution(0, _config.limit, _config.skew) {}

            size_t nextOffset(void) {
                return distribution.nextValue() * config.block_size;
            }
    };

    void to_json(json& j, const ZipfianAccessPatternConfig& config) {
        j = json{
            {"type", "zipfian"},
            {"block_size", config.block_size},
            {"limit", config.limit},
            {"skew", config.skew},
        };
    }

    void from_json(const json& j, ZipfianAccessPatternConfig& config) {
        j.at("block_size").get_to(config.block_size);
        j.at("limit").get_to(config.limit);
        j.at("skew").get_to(config.skew);
        config.validate();
        config.limit = config.limit / config.block_size - 1;
    }
}

#endif