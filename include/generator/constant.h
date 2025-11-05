#ifndef CONSTANT_GENERATOR_H
#define CONSTANT_GENERATOR_H

#include <cstddef>
#include <cstdint>
#include <cstring>

namespace Generator {
    class ConstantGenerator {
        public:
            void nextBlock(uint8_t* buffer, size_t size);
    };
};

#endif