#ifndef ACCESS_PATTERN_H
#define ACCESS_PATTERN_H

#include <cstddef>
#include <distribution/distribution.h>

namespace AccessPattern {
    inline size_t maxBlockIndex(size_t limit, size_t block_size) {
        assert(block_size > 0 && "block_size must be greater than 0");
        assert(block_size < limit && "block_size must be less than limit");
        return (limit % block_size == 0)
            ? (limit / block_size) - 1
            : (limit / block_size);
    }

    struct SequentialAccessPattern {
        size_t current;
        size_t block_size;
        size_t limit;

        constexpr inline SequentialAccessPattern(size_t limit, size_t block_size)
            : current(0), block_size(block_size), limit(limit) {
                assert(block_size > 0 && "block_size must be greater than 0");
                assert(block_size < limit && "block_size must be less than limit");
            }

        constexpr inline size_t nextOffset() {
            size_t offset = current;
            current = (current + block_size) % limit;
            return offset;
        }
    };

    struct RandomAccessPattern {
        size_t block_size;
        Distribution::UniformDistribution<size_t> distribution;

        inline RandomAccessPattern(size_t limit, size_t block_size)
            : block_size(block_size), distribution(0, maxBlockIndex(limit, block_size)) {}

        constexpr inline size_t nextOffset() {
            return distribution.nextValue() * block_size;
        }        
    };

    struct ZipfianAccessPattern {
        size_t block_size;
        Distribution::ZipfianDistribution<size_t> distribution;

        inline ZipfianAccessPattern(size_t limit, size_t block_size, float skew)
            : block_size(block_size), distribution(0, maxBlockIndex(limit, block_size), skew) {}

        constexpr inline size_t nextOffset() {
            return distribution.nextValue() * block_size;
        }
    };   
}

#endif