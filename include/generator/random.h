#ifndef RANDOM_GENERATOR_H
#define RANDOM_GENERATOR_H

#include <lib/shishua/shishua.h>

namespace Generator {

    class RandomGenerator {
        private:
            prng_state generator;

        public:
            explicit RandomGenerator();
            void nextBlock(uint8_t* buffer, size_t size);
    };
};

#endif