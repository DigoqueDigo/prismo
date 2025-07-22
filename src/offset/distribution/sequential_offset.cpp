#include <offset/distribution/sequential_offset.h>

SequentialOffset::SequentialOffset(int limit, int block_size)
    : OffsetDistribution(limit, block_size), current_offset(0) {}

int SequentialOffset::sample() {
    int offset = this->current_offset;
    this->current_offset = (this->current_offset + this->block_size) % this->limit;
    return offset;
}
