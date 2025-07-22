#include <offset/distribution/binomial_offset.h>

BinomialOffset::BinomialOffset(int limit, int block_size, float probability)
    : OffsetDistribution(limit, block_size),
        rng(std::random_device{}()),
        dist((limit / block_size) - 1, probability) {}

int BinomialOffset::sample() {
    return this->dist(this->rng) * this->block_size;
}
