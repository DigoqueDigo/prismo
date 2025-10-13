#ifndef CONSTANT_GENERATOR_H
#define CONSTANT_GENERATOR_H

#include <generator/block.h>

namespace Generator {

    struct ConstantGenerator {
        public:
            void nextBlock(Block& block);
    };
};

#endif