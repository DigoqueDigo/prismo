#include <generator/random_generator.h>

RandomGenerator::RandomGenerator(size_t block_size)
    : BlockGenerator(block_size), rng(std::random_device{}()), dist(0, 255) {}

void RandomGenerator::next(std::vector<std::byte>& block) {
    std::generate(block.begin(), block.end(), [this]() {
        return static_cast<std::byte>(this->dist(this->rng));
    });
}