#include <generator/constant.h>

namespace Generator {
    
    void ConstantGenerator::nextBlock(Block& block) {
        std::memset(block.getBuffer(), 0, block.getSize());
    }
}