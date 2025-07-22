#ifndef BINOMIAL_OFFSET_H
#define BINOMIAL_OFFSET_H

#include <random>
#include <offset/distribution/offset_distribution.h>

class BinomialOffset : public OffsetDistribution {

private:
    std::mt19937 rng;
    std::binomial_distribution<int> dist;

public:
    BinomialOffset() = delete;
    explicit BinomialOffset(int limit, int block_size, float probability);
    ~BinomialOffset() override = default;
    int sample() override;
};

#endif