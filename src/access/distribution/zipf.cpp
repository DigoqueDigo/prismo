#include <access/distribution/zipf.h>

Zipf::Zipf(int limit, int block_size, double theta)
    : Distribution(limit, block_size),
        rng(std::random_device{}()),
        dist(0, (limit / block_size) - 1, theta) {}

int Zipf::sample() {
    return this->dist(this->rng) * this->block_size;
}