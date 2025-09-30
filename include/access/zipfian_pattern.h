#ifndef ZIPFIAN_ACCESS_PATTERN_H
#define ZIPFIAN_ACCESS_PATTERN_H

#include <cstddef>
#include <stdexcept>
#include <nlohmann/json.hpp>
#include <distribution/distribution.h>

using json = nlohmann::json;

namespace AccessPattern {
    struct ZipfianAccessPattern {
        size_t block_size;
        size_t limit;
        float skew;
        Distribution::ZipfianDistribution<unsigned long> distribution;

        explicit ZipfianAccessPattern()
            : block_size(0), limit(0), skew(), distribution() {}

        // explicit ZipfianAccessPattern(size_t _limit, size_t _block_size, float _skew)
        //     : block_size(_block_size), distribution(0, maxBlockIndex(_limit, _block_size), _skew) {}

        unsigned long nextOffset() {
            return distribution.nextValue() * block_size;
        }
    };

        void to_json(json& j, const ZipfianAccessPattern& zipfian_pattern) {
        j = json{
            {"type", "zipfian"},
            {"block_size", zipfian_pattern.block_size},
            {"limit", zipfian_pattern.limit},
            {"skew", zipfian_pattern.skew},
        };
    }

    void from_json(const json& j, ZipfianAccessPattern& zipfian_pattern) {
        if (j.at("type").template get<std::string>() != "zipfian") {
            throw std::runtime_error("Invalid JSON type for ZipfianAccessPattern");
        }

        j.at("block_size").get_to(zipfian_pattern.block_size);
        if (zipfian_pattern.block_size == 0) {
            throw std::invalid_argument("Invalid JSON block_size for ZipfianAccessPattern");
        }

        j.at("limit").get_to(zipfian_pattern.limit);
        if (zipfian_pattern.block_size > zipfian_pattern.limit) {
            throw std::invalid_argument("Invalid JSON limit for ZipfianAccessPattern");
        }

        j.at("skew").get_to(zipfian_pattern.skew);
        if (zipfian_pattern.skew < 0 || zipfian_pattern.skew > 1) {
            throw std::invalid_argument("Invalid JSON skew for ZipfianAccessPattern");
        }

        zipfian_pattern.distribution.setParams(
            0,
            (zipfian_pattern.limit % zipfian_pattern.block_size == 0)
            ? static_cast<unsigned long>((zipfian_pattern.limit / zipfian_pattern.block_size) - 1)
            : static_cast<unsigned long>((zipfian_pattern.limit / zipfian_pattern.block_size)),
            zipfian_pattern.skew
        );
    }
}

#endif