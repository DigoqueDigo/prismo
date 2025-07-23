#ifndef BLOCK_GENERATOR_H
#define BLOCK_GENERATOR_H

#include <vector>
#include <stdexcept>

class BlockGenerator {

protected:
    size_t block_size;

public:
    BlockGenerator() = delete;
    explicit BlockGenerator(size_t size);
    virtual ~BlockGenerator() = default;
    virtual void next(std::vector<std::byte> &block) = 0;
};

#endif