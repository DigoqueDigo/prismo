#include <generator/constant.h>

namespace Generator {
    void ConstantGenerator::nextBlock(uint8_t* buffer, size_t size) {
        std::memset(buffer, 0, size);
    }
}