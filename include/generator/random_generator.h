#ifndef RANDOM_BLOCK_GENERATOR_H
#define RANDOM_BLOCK_GENERATOR_H

#include <vector>
#include <cstddef>
#include <cstring>
#include <stdexcept>
#include <iostream>
#include <distribution/distribution.h>

namespace BlockGenerator {
    struct ConstantBlockGeneratorConfig {
        size_t block_size;

        void validate(void) {
            if (block_size == 0 || block_size % sizeof(u_int64_t) != 0)
                throw std::invalid_argument("Invalid block_size for RandomBlockGeneratorConfig");
        }
    };

    struct RandomBlockGenerator {
        Block
        Distribution::UniformDistribution<uint64_t> distribution;

        explicit RandomBlockGenerator(size_t block_size)
            : data_buffer(block_size / sizeof(uint64_t)), distribution() {
                if (block_size % sizeof(uint64_t) != 0) {
                    throw std::invalid_argument("Block size must be a multiple of 8 bytes");
                }
            }

        void fillBlock(Block& block) {
            unsigned int capacity = static_cast<unsigned int>(data_buffer.capacity()); 
            for (unsigned int iter = 0; iter < capacity; iter++) {
                data_buffer[iter] = distribution.nextValue();
            }
            block.fillBuffer(data_buffer.data(), block.size);
        }
    };
};

#endif