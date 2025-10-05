#ifndef SEQUENTIAL_ACCESS_PATTERN_H
#define SEQUENTIAL_ACCESS_PATTERN_H

#include <cstddef>
#include <stdexcept>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace AccessPattern {
    struct SequentialAccessPatternConfig {
        size_t block_size;
        size_t limit;

        void validate(void) const {
            if (block_size == 0)
                throw std::invalid_argument("Invalid block_size for SequentialAccessPatternConfig");
            if (block_size > limit)
                throw std::invalid_argument("Invalid limit for SequentialAccessPatternConfig");
        }
    };

    struct SequentialAccessPattern {
        private:
            const SequentialAccessPatternConfig config;
            size_t current_offset;

        public:
            explicit SequentialAccessPattern(const SequentialAccessPatternConfig& _config)
                : config(_config), current_offset(0) {}

            size_t nextOffset(void) {
                const size_t offset = current_offset;
                current_offset = (current_offset + config.block_size) % config.limit;
                return offset;
            }
    };

    void from_json(const json& j, SequentialAccessPatternConfig& config) {
        j.at("block_size").get_to(config.block_size);
        j.at("limit").get_to(config.limit);
        config.validate();
        config.limit = config.block_size * (config.limit / config.block_size);
    }
}

#endif