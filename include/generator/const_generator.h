#ifndef CONST_BLOCK_GENERATOR_H
#define CONST_BLOCK_GENERATOR_H

#include <generator/block_generator.h>

class ConstGenerator : public BlockGenerator {

private:
    std::vector<std::byte> const_block;

public:
    ConstGenerator() = delete;
    explicit ConstGenerator(size_t block_size, std::byte value = std::byte{0});
    ~ConstGenerator() override = default;
    void next(std::vector<std::byte> &block) override;
};

#endif