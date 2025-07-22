#ifndef ZIPF_H
#define ZIPF_H

#include <access/distribution/distribution.h>
#include <access/distribution/implementation/zipf.h>

class Zipf : public Distribution {

private:
    std::mt19937 rng;
    zipfian_int_distribution<int> dist;

public:
    Zipf() = delete;
    Zipf(int limit, int block_size, double theta);
    ~Zipf() override = default;
    int sample() override;
};

#endif