#ifndef RANDOM_BLOCK_GENERATOR_H
#define RANDOM_BLOCK_GENERATOR_H

#include <generator/block.h>
#include <lib/shishua/shishua.h>
#include <lib/distribution/distribution.h>

namespace BlockGenerator {
    struct RandomBlockGenerator {
        prng_state generator;

        explicit RandomBlockGenerator() {
            uint64_t seed[4] = {0, 0, 0, 0}; 
            prng_init(&generator, seed);
        };

        void nextBlock(Block& block) {
            prng_gen(&generator, block.buffer, block.config.size);
        }
    };
};

#endif