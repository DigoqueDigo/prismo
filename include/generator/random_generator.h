#ifndef RANDOM_BLOCK_GENERATOR_H
#define RANDOM_BLOCK_GENERATOR_H

#include <random>
#include <algorithm>
#include <generator/block_generator.h>

class RandomGenerator : public BlockGenerator {

private:
    std::mt19937 rng;
    std::uniform_int_distribution<unsigned char> dist;

public:
    RandomGenerator() = delete;
    explicit RandomGenerator(size_t block_size);
    ~RandomGenerator() override = default;
    void next(std::vector<std::byte> &block) override;
};

#endif  