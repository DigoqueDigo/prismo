#ifndef ZIPF_OFFSET_H
#define ZIPF_OFFSET_H

#include <offset/distribution/offset_distribution.h>
#include <offset/distribution/implementation/zipf.h>

class ZipfOffset : public OffsetDistribution {

private:
    std::mt19937 rng;
    zipfian_int_distribution<int> dist;

public:
    ZipfOffset() = delete;
    explicit ZipfOffset(size_t limit, size_t block_size, double theta);
    ~ZipfOffset() override = default;
    size_t sample() override;
};

#endif