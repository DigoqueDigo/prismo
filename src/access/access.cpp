#include <access/access.h>

Access::Access(int size_limit, int block_size) : 
    size_limit(size_limit), block_size(block_size) {}

Access::~Access() {}