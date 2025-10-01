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

        void validate(void) {
            if (block_size == 0)
                throw std::invalid_argument("Invalid block_size for SequentialAccessPatternConfig");
            if (block_size > limit)
                throw std::invalid_argument("Invalid limit for SequentialAccessPatternConfig");
        }
    };

    struct SequentialAccessPattern {
        const SequentialAccessPatternConfig config;
        unsigned long current_offset;

        explicit SequentialAccessPattern(const SequentialAccessPatternConfig& _config)
            : config(_config), current_offset(0) {}

        constexpr unsigned long nextOffset() {
            const unsigned long offset = current_offset;
            current_offset = (current_offset + config.block_size) % config.limit;
            return offset;
        }
    };

    void to_json(json& j, const SequentialAccessPatternConfig& config) {
        j = json{
            {"type", "sequential"},
            {"block_size", config.block_size},
            {"limit", config.limit}
        };
    }

    void from_json(const json& j, SequentialAccessPatternConfig& config) {
        j.at("block_size").get_to(config.block_size);
        j.at("limit").get_to(config.limit);
        config.validate();
    }
}

#endif