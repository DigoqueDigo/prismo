#ifndef CONSTANT_GENERATOR_H
#define CONSTANT_GENERATOR_H

#include <generator/block.h>

namespace Generator {
    struct ConstantGenerator {
        void nextBlock(Block& block) {
            std::memset(block.buffer, 0, block.config.size);
        }
    };
};

#endif