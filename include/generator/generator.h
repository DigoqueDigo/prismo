#ifndef GENERATOR_H
#define GENERATOR_H

#include <vector>
#include <cstddef>
#include <cstring>
#include <stdexcept>
#include <iostream>
#include <distribution/distribution.h>

namespace BlockGenerator {
    struct Block {
        const size_t size;
        std::vector<std::byte> buffer;

        explicit Block(size_t size)
            : size(size), buffer(size, std::byte(0)) {}

        inline void fillBuffer(const void* data, size_t size) {
            std::memcpy(buffer.data(), data, size);
        }
    };

    struct RandomBlockGenerator {
        std::vector<uint64_t> data_buffer;
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