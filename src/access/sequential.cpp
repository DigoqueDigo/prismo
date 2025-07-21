#include <access/sequential.h>

Sequential::Sequential(int size_limit, int block_size) :
    Access(size_limit, block_size), current_offset() {}

Sequential::~Sequential() {}

int Sequential::next(void) {
    int offset = this->current_offset;
    this->current_offset += this->block_size;
    this->current_offset %= this->size_limit;
    return offset;
}