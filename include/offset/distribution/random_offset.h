#ifndef RANDOM_OFFSET_H
#define RANDOM_OFFSET_H

#include <random>
#include <offset/distribution/offset_distribution.h>

class RandomOffset : public OffsetDistribution {

private:
    std::mt19937 rng;
    std::uniform_int_distribution<int> dist;

public:
    RandomOffset() = delete;
    explicit RandomOffset(int limit, int block_size);
    ~RandomOffset() override = default;
    int sample() override;
};

#endif