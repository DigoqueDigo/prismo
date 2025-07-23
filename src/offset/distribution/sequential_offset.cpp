#include <offset/distribution/sequential_offset.h>

SequentialOffset::SequentialOffset(size_t limit, size_t block_size)
    : OffsetDistribution(limit, block_size), current_offset(0) {}

size_t SequentialOffset::sample() {
    int offset = this->current_offset;
    this->current_offset = (this->current_offset + this->block_size) % this->limit;
    return offset;
}
