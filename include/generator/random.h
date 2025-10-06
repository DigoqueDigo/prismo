#ifndef RANDOM_GENERATOR_H
#define RANDOM_GENERATOR_H

#include <generator/block.h>
#include <lib/shishua/shishua.h>
#include <lib/distribution/distribution.h>

namespace Generator {
    struct RandomGenerator {
        prng_state generator;

        explicit RandomGenerator() {
            uint64_t seed[4] = {0, 0, 0, 0}; 
            prng_init(&generator, seed);
        };

        void nextBlock(Block& block) {
            prng_gen(&generator, block.buffer, block.config.size);
        }
    };
};

#endif