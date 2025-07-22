#include <access/distribution/random.h>

Random::Random(int limit, int block_size)
    : Distribution(limit, block_size),
        rng(std::random_device{}()),
        dist(0, (limit / block_size) - 1) {}

int Random::sample() {
    return this->dist(this->rng) * this->block_size;
}