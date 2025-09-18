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
        return (limit % block_size == 0)
            ? (limit / block_size) - 1
            : (limit / block_size);
    }

    struct SequentialAccessPattern {
        unsigned long current_offset;
        const size_t block_size;
        const size_t limit;

        explicit SequentialAccessPattern(size_t _limit, size_t _block_size)
            : current_offset(0), block_size(_block_size), limit(maxBlockIndex(_limit, _block_size) * _block_size) {}

        constexpr unsigned long nextOffset() {
            const unsigned long offset = current_offset;
            current_offset = (current_offset + block_size) % limit;
            return offset;
        }
    };

    struct RandomAccessPattern {
        const size_t block_size;
        Distribution::UniformDistribution<unsigned long> distribution;

        explicit RandomAccessPattern(size_t _limit, size_t _block_size)
            : block_size(_block_size), distribution(0, maxBlockIndex(_limit, _block_size)) {}

        unsigned long nextOffset() {
            return distribution.nextValue() * block_size;
        }
    };

    struct ZipfianAccessPattern {
        const size_t block_size;
        Distribution::ZipfianDistribution<unsigned long> distribution;

        explicit ZipfianAccessPattern(size_t _limit, size_t _block_size, float _skew)
            : block_size(_block_size), distribution(0, maxBlockIndex(_limit, _block_size), _skew) {}

        unsigned long nextOffset() {
            return distribution.nextValue() * block_size;
        }
    };
}

#endif