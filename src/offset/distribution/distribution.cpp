#include <access/distribution/distribution.h>

Distribution::Distribution(int limit, int block_size)
    : limit(limit), block_size(block_size) {
        if (limit <= 0 || block_size <= 0 || limit % block_size != 0) {
            throw std::invalid_argument("Invalid distribution parameters");
        }
    }