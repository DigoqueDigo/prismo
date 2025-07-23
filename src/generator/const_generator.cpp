#include <generator/const_generator.h>

ConstGenerator::ConstGenerator(size_t block_size, std::byte value)
    : BlockGenerator(block_size), const_block(block_size, value) {}

void ConstGenerator::next(std::vector<std::byte> &block) {
    block.assign(this->const_block.begin(), this->const_block.end());
}