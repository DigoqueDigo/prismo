#ifndef ACCESS_PATTERN_H
#define ACCESS_PATTERN_H

#include <cstddef>
#include <stdexcept>
#include <sys/types.h>
#include <distribution/distribution.h>

// TODO:: does the access pattern need to be aligned to block size?

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
        off_t current_offset;
        const size_t block_size;
        const size_t limit;

        explicit SequentialAccessPattern(size_t _limit, size_t _block_size)
            : current_offset(0), block_size(_block_size), limit(maxBlockIndex(_limit, _block_size) * _block_size) {}
            // TODO: fix if previous TODO is incorrect

        constexpr off_t nextOffset() {
            const off_t offset = current_offset;
            current_offset = (current_offset + block_size) % limit;
            return offset;
        }
    };

    struct RandomAccessPattern {
        const size_t block_size;
        Distribution::UniformDistribution<off_t> distribution;

        explicit RandomAccessPattern(size_t _limit, size_t _block_size)
            : block_size(_block_size), distribution(0, maxBlockIndex(_limit, _block_size)) {}

        off_t nextOffset() {
            return distribution.nextValue() * block_size;
        }
    };

    struct ZipfianAccessPattern {
        const size_t block_size;
        Distribution::ZipfianDistribution<off_t> distribution;

        explicit ZipfianAccessPattern(size_t _limit, size_t _block_size, float _skew)
            : block_size(_block_size), distribution(0, maxBlockIndex(_limit, _block_size), _skew) {}

        off_t nextOffset() {
            return distribution.nextValue() * block_size;
        }
    };
}

#endif