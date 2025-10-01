#ifndef RANDOM_ACCESS_PATTERN_H
#define RANDOM_ACCESS_PATTERN_H

#include <cstddef>
#include <stdexcept>
#include <nlohmann/json.hpp>
#include <distribution/distribution.h>

using json = nlohmann::json;

namespace AccessPattern {
    struct RandomAccessPatternConfig {
        size_t block_size;
        size_t limit;
        size_t distribution_max_limit;

        void validate(void) {
            if (block_size == 0)
                throw std::invalid_argument("Invalid block_size for RandomAccessPatternConfig");
            if (block_size > limit)
                throw std::invalid_argument("Invalid limit for RandomAccessPatternConfig");
        }
    };

    struct RandomAccessPattern {
        const RandomAccessPatternConfig config;
        Distribution::UniformDistribution<unsigned long> distribution;

        explicit RandomAccessPattern(const RandomAccessPatternConfig& _config)
            : config(_config), distribution(0, _config.distribution_max_limit) {}

        unsigned long nextOffset() {
            return distribution.nextValue() * config.block_size;
        }
    };

    void to_json(json& j, const RandomAccessPatternConfig& config) {
        j = json{
            {"type", "random"},
            {"block_size", config.block_size},
            {"limit", config.limit}
        };
    }

    void from_json(const json& j, RandomAccessPatternConfig& config) {
        j.at("block_size").get_to(config.block_size);        
        j.at("limit").get_to(config.limit);
        config.distribution_max_limit = (config.limit % config.block_size == 0)
            ? (config.limit / config.block_size) - 1
            : (config.limit / config.block_size);
        config.validate();
    }
}

#endif