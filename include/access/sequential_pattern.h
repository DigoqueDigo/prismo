#ifndef SEQUENTIAL_ACCESS_PATTERN_H
#define SEQUENTIAL_ACCESS_PATTERN_H

#include <cstddef>
#include <stdexcept>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace AccessPattern {
    struct SequentialAccessPattern {
        unsigned long current_offset;
        size_t block_size;
        size_t limit;

        explicit SequentialAccessPattern()
            : current_offset(0), block_size(0), limit(0) {}

        // explicit SequentialAccessPattern(size_t _limit, size_t _block_size)
        //     : current_offset(0), block_size(_block_size), limit(_limit) {
        //         if (block_size == 0 || block_size > limit) {
        //             throw std::invalid_argument("AccessPattern :: invalid block size: " + std::to_string(block_size));
        //         }
        //     }

        constexpr unsigned long nextOffset() {
            const unsigned long offset = current_offset;
            current_offset = (current_offset + block_size) % limit;
            return offset;
        }
    };

    void to_json(json& j, const SequentialAccessPattern& sequential_pattern) {
        j = json{
            {"type", "sequential"},
            {"block_size", sequential_pattern.block_size},
            {"limit", sequential_pattern.limit}
        };
    }

    void from_json(const json& j, SequentialAccessPattern& sequential_pattern) {
        if (j.at("type").template get<std::string>() != "sequential") {
            throw std::runtime_error("Invalid JSON type for SequentialAccessPattern");
        }

        j.at("block_size").get_to(sequential_pattern.block_size);
        if (sequential_pattern.block_size == 0) {
            throw std::invalid_argument("Invalid JSON block_size for SequentialAccessPattern");
        }

        j.at("limit").get_to(sequential_pattern.limit);
        if (sequential_pattern.block_size > sequential_pattern.limit) {
            throw std::invalid_argument("Invalid JSON limit for SequentialAccessPattern");
        }
    }
}

#endif