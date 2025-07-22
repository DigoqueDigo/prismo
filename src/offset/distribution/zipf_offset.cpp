#include <offset/distribution/zipf_offset.h>

ZipfOffset::ZipfOffset(int limit, int block_size, double theta)
    : OffsetDistribution(limit, block_size),
        rng(std::random_device{}()),
        dist(0, (limit / block_size) - 1, theta) {}

int ZipfOffset::sample() {
    return this->dist(this->rng) * this->block_size;
}