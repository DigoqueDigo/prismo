#ifndef GENERATOR_H
#define GENERATOR_H

#include <vector>
#include <cstddef>
#include <stdexcept>
#include <distribution/distribution.h>

namespace BlockGenerator {
    struct Block {
        unsigned long id;
        const size_t length;
        std::vector<uint64_t> data;

        explicit Block(size_t size)
            : id(0), length(size / sizeof(uint64_t)), data(size / sizeof(uint64_t), 0ULL) {
                if (size % sizeof(uint64_t) != 0) {
                    throw std::invalid_argument("Block size must be a multiple of " + std::to_string(sizeof(uint64_t)));
                }
            }

        constexpr inline void incrementId() {
            id += 1;
        }

        inline void setDataAt(uint64_t value, size_t index) {
            data[index] = value;
        }
    };

    struct RandomBlockGenerator {
        Distribution::UniformDistribution<uint64_t> distribution;

        explicit RandomBlockGenerator()
            : distribution() {}

        void fillBlock(Block& block) {
            block.incrementId();
            for (size_t index = 0; index < block.length; index++) {
                block.setDataAt(distribution.nextValue(), index);
            }
        }
    };
};

#endif