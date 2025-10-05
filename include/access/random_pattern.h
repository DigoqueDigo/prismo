#ifndef RANDOM_ACCESS_PATTERN_H
#define RANDOM_ACCESS_PATTERN_H

#include <cstddef>
#include <stdexcept>
#include <nlohmann/json.hpp>
#include <lib/distribution/distribution.h>

using json = nlohmann::json;

namespace AccessPattern {
    struct RandomAccessPatternConfig {
        size_t block_size;
        size_t limit;

        void validate(void) const {
            if (block_size == 0)
                throw std::invalid_argument("Invalid block_size for RandomAccessPatternConfig");
            if (block_size > limit)
                throw std::invalid_argument("Invalid limit for RandomAccessPatternConfig");
        }
    };

    struct RandomAccessPattern {
        private:
            const RandomAccessPatternConfig config;
            Distribution::UniformDistribution<size_t> distribution;

        public:
            explicit RandomAccessPattern(const RandomAccessPatternConfig& _config)
                : config(_config), distribution(0, _config.limit) {}

            size_t nextOffset(void) {
                return distribution.nextValue() * config.block_size;
            }
    };

    void from_json(const json& j, RandomAccessPatternConfig& config) {
        j.at("block_size").get_to(config.block_size);
        j.at("limit").get_to(config.limit);
        config.validate();
        config.limit = config.limit / config.block_size - 1;
    }
}

#endif