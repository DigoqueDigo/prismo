#ifndef OFFSET_DISTRIBUTION_H
#define OFFSET_DISTRIBUTION_H

#include <stdexcept>

class OffsetDistribution {

protected:
    const size_t limit;
    const size_t block_size;

public:
    OffsetDistribution() = delete;
    explicit OffsetDistribution(size_t limit, size_t block_size);
    virtual ~OffsetDistribution() = default;
    virtual size_t sample() = 0;
};

#endif