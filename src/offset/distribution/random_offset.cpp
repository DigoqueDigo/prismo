#include <offset/distribution/random_offset.h>

RandomOffset::RandomOffset(int limit, int block_size)
    : OffsetDistribution(limit, block_size),
        rng(std::random_device{}()),
        dist(0, (limit / block_size) - 1) {}

int RandomOffset::sample() {
    return this->dist(this->rng) * this->block_size;
}