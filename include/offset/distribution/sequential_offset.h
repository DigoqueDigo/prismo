#ifndef SEQUENTIAL_OFFSET_H
#define SEQUENTIAL_OFFSET_H

#include <offset/distribution/offset_distribution.h>

class SequentialOffset : public OffsetDistribution {

private:
    int current_offset;

public:
    SequentialOffset() = delete;
    explicit SequentialOffset(size_t limit, size_t block_size);
    ~SequentialOffset() override = default;
    size_t sample() override;
};

#endif