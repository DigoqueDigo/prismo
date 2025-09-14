#ifndef ACCESS_PATTERN_H
#define ACCESS_PATTERN_H

#include <cstddef>
#include <stdexcept>
#include <distribution/distribution.h>

namespace AccessPattern {
    inline size_t maxBlockIndex(size_t limit, size_t block_size) {
        if (block_size == 0)
            throw std::invalid_argument("AccessPattern :: block_size must be greater than 0");
        if (block_size >= limit) 
            throw std::invalid_argument("AccessPattern :: block_size must be less than limit");
        return (limit % block_size == 0) // TODO: check if this is correct (should only accept if modulo is 0)
            ? (limit / block_size) - 1
            : (limit / block_size);
    }

    struct SequentialAccessPattern {
        size_t current;
        const size_t block_size;
        const size_t limit;

        explicit SequentialAccessPattern(size_t limit, size_t block_size)
            : current(0), block_size(block_size), limit(maxBlockIndex(limit, block_size) * block_size) {}
            // TODO: fix if previous TODO is incorrect

        constexpr size_t nextOffset() {
            const size_t offset = current;
            current = (current + block_size) % limit;
            return offset;
        }
    };

    struct RandomAccessPattern {
        const size_t block_size;
        Distribution::UniformDistribution<size_t> distribution;

        explicit RandomAccessPattern(size_t limit, size_t block_size)
            : block_size(block_size), distribution(0, maxBlockIndex(limit, block_size)) {}

        size_t nextOffset() {
            return distribution.nextValue() * block_size;
        }
    };

    struct ZipfianAccessPattern {
        const size_t block_size;
        Distribution::ZipfianDistribution<size_t> distribution;

        explicit ZipfianAccessPattern(size_t limit, size_t block_size, float skew)
            : block_size(block_size), distribution(0, maxBlockIndex(limit, block_size), skew) {}

        size_t nextOffset() {
            return distribution.nextValue() * block_size;
        }
    };
}

#endif