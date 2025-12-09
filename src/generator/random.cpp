#include <generator/synthetic.h>

namespace Generator {

    RandomGenerator::RandomGenerator()
        : Generator() {
            auto seed = generate_seed();
            prng_init(&generator, seed.data());
    };

    void RandomGenerator::nextBlock(uint8_t* buffer, size_t size) {
        prng_gen(&generator, buffer, size);
    }
}