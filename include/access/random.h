#ifndef RANDOM_ACCESS_H
#define RANDOM_ACCESS_H

#include <cstddef>
#include <nlohmann/json.hpp>
#include <lib/distribution/distribution.h>

using json = nlohmann::json;

namespace Access {

    struct RandomAccessConfig {
        private:
            size_t block_size;
            size_t limit;

        public:
            constexpr inline size_t getBlockSize(void) const { return block_size; }
            constexpr inline size_t getLimit(void) const { return limit; }

            void validate(void) const;
            friend void from_json(const json& j, RandomAccessConfig& config);
    };

    struct RandomAccess {
        private:
            const RandomAccessConfig config;
            Distribution::UniformDistribution<size_t> distribution;

        public:
            explicit RandomAccess(const RandomAccessConfig& _config);
            size_t nextOffset(void);
    };

    void from_json(const json& j, RandomAccessConfig& config);
}

#endif
