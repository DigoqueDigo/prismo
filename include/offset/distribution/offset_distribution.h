#ifndef OFFSET_DISTRIBUTION_H
#define OFFSET_DISTRIBUTION_H

#include <stdexcept>

class OffsetDistribution {

protected:
    const int limit;
    const int block_size;

public:
    OffsetDistribution() = delete;
    explicit OffsetDistribution(int limit, int block_size);
    virtual ~OffsetDistribution() = default;
    virtual int sample() = 0;
};

#endif