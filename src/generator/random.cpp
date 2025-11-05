#include <generator/random.h>

namespace Generator {

    RandomGenerator::RandomGenerator() {
        uint64_t seed[4] = {0, 0, 0, 0};
        prng_init(&generator, seed);
    };

    void RandomGenerator::nextBlock(uint8_t* buffer, size_t size) {
        prng_gen(&generator, buffer, size);
    }
}