#ifndef ACCESS_PATTERN_H
#define ACCESS_PATTERN_H

#include <cstddef>
#include <distribution/distribution.h>

namespace AccessPattern {
    struct SequentialPattern {
        size_t current;
        size_t block_size;
        size_t limit;

        constexpr inline SequentialPattern(size_t limit, size_t block_size)
            : current(0), block_size(block_size), limit(limit) {}

        constexpr inline size_t nextOffset() {
            size_t offset = current;
            current = (current + block_size) % limit;
            return offset;
        }
    };

    struct RandomPattern {
        size_t block_size;
        Distribution::UniformDistribution<size_t> distribution;

        inline RandomPattern(size_t limit, size_t block_size)
            : block_size(block_size), distribution(0, limit / block_size) {}

        constexpr inline size_t nextOffset() {
            return distribution.nextValue() * block_size;
        }        
    };
}

#endif