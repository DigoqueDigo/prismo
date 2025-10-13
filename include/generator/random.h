#ifndef RANDOM_GENERATOR_H
#define RANDOM_GENERATOR_H

#include <generator/block.h>
#include <lib/shishua/shishua.h>

namespace Generator {

    struct RandomGenerator {
        private:
            prng_state generator;

        public:
            explicit RandomGenerator();
            void nextBlock(Block& block);
    };
};

#endif