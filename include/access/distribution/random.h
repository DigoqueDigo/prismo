#ifndef RANDOM_H
#define RANDOM_H

#include <random>
#include <access/distribution/distribution.h>

class Random : public Distribution {

private:
    std::mt19937 rng;
    std::uniform_int_distribution<int> dist;

public:
    Random() = delete;
    Random(int limit, int block_size);
    ~Random() override = default;
    int sample() override;
};

#endif