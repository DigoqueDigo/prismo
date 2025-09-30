#ifndef RANDOM_ACCESS_PATTERN_H
#define RANDOM_ACCESS_PATTERN_H

#include <cstddef>
#include <stdexcept>
#include <nlohmann/json.hpp>
#include <distribution/distribution.h>

using json = nlohmann::json;

namespace AccessPattern {
    struct RandomAccessPattern {
        size_t block_size;
        size_t limit;
        Distribution::UniformDistribution<unsigned long> distribution;

        explicit RandomAccessPattern()
            : block_size(0), limit(0), distribution() {}

        // explicit RandomAccessPattern(size_t _limit, size_t _block_size)
        //     : block_size(_block_size), limit(_limit), distribution(0, maxBlockIndex(_limit, _block_size)) {}

        unsigned long nextOffset() {
            return distribution.nextValue() * block_size;
        }
    };

    void to_json(json& j, const RandomAccessPattern& random_pattern) {
        j = json{
            {"type", "random"},
            {"block_size", random_pattern.block_size},
            {"limit", random_pattern.limit}
        };
    }

    void from_json(const json& j, RandomAccessPattern& random_pattern) {
        if (j.at("type").template get<std::string>() != "random") {
            throw std::runtime_error("Invalid JSON type for RandomAccessPattern");
        }

        j.at("block_size").get_to(random_pattern.block_size);
        if (random_pattern.block_size == 0) {
            throw std::invalid_argument("Invalid JSON block_size for RandomAccessPattern");
        }

        j.at("limit").get_to(random_pattern.limit);
        if (random_pattern.block_size > random_pattern.limit) {
            throw std::invalid_argument("Invalid JSON limit for RandomAccessPattern");
        }

        random_pattern.distribution.setParams(
            0,
            (random_pattern.limit % random_pattern.block_size == 0)
            ? static_cast<unsigned long>((random_pattern.limit / random_pattern.block_size) - 1)
            : static_cast<unsigned long>((random_pattern.limit / random_pattern.block_size))
        );
    }
}

#endif