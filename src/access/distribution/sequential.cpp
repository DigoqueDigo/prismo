#include <access/distribution/sequential.h>

Sequential::Sequential(int limit, int block_size)
    : Distribution(limit, block_size), current_offset(0) {}

int Sequential::sample() {
    int offset = this->current_offset;
    this->current_offset += this->block_size;
    this->current_offset %= this->limit;
    return offset;
}
