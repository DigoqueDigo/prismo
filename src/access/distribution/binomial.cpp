#include <access/distribution/binomial.h>

Binomial::Binomial(int limit, int block_size, float probability)
    : Distribution(limit, block_size),
        rng(std::random_device{}()),
        dist(limit / block_size, probability) {}

int Binomial::sample() {
    return this->dist(this->rng) * this->block_size;
}
