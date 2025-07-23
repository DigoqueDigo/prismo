#include <generator/const_generator.h>

BlockGenerator::BlockGenerator(size_t block_size)
    : block_size(block_size) {
        if (block_size == 0) {
            throw std::invalid_argument("Block size must be greater than zero.");
        }
    }