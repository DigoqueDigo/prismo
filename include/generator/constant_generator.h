#ifndef CONSTANT_BLOCK_GENERATOR_H
#define CONSTANT_BLOCK_GENERATOR_H

#include <cstdint>
#include <cstring>
#include <generator/block.h>

namespace BlockGenerator {
    struct ConstantBlockGenerator {
        void nextBlock(Block& block) {
            std::memset(block.buffer, 0, block.config.size);
        }
    };
};

#endif