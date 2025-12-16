#include <generator/synthetic.h>

namespace Generator {

    RandomGenerator::RandomGenerator()
        : Generator() {
            auto seed = generate_seed();
            prng_init(&generator, seed.data());
    };

    BlockMetadata RandomGenerator::next_block(uint8_t* buffer, size_t size) {
        prng_gen(&generator, buffer, size);
        std::memcpy(buffer, &block_id, sizeof(block_id));
        return BlockMetadata {
            .block_id = block_id++,
            .compression = 0
        };
    }
}