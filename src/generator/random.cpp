#include <generator/random.h>

namespace Generator {

    RandomGenerator::RandomGenerator() {
        uint64_t seed[4] = {0, 0, 0, 0}; 
        prng_init(&generator, seed);
    };

    void RandomGenerator::nextBlock(Block& block) {
        prng_gen(&generator, block.getBuffer(), block.getSize());
    }
}