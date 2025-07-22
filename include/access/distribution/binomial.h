#ifndef BINOMIAL_H
#define BINOMIAL_H

#include <random>
#include <access/distribution/distribution.h>

class Binomial : public Distribution {

private:
    std::mt19937 rng;
    std::binomial_distribution<int> dist;

public:
    Binomial() = delete;
    explicit Binomial(int limit, int block_size, float probability);
    ~Binomial() override = default;
    int sample() override;
};

#endif