#include <generator/synthetic.h>

namespace Generator {

    RandomGenerator::RandomGenerator()
        : Generator() {
            auto seed = generate_seed();
            prng_init(&generator, seed.data());
    };

    uint64_t RandomGenerator::nextBlock(uint8_t* buffer, size_t size) {
        prng_gen(&generator, buffer, size);
        std::memcpy(buffer, &block_id, sizeof(block_id));
        return block_id++;
    }
}