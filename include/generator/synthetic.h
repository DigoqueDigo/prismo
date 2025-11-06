#ifndef SYNTHETIC_GENERATOR_H
#define SYNTHETIC_GENERATOR_H

#include <cstdint>
#include <cstddef>
#include <cstring>
#include <lib/shishua/shishua.h>

namespace Generator {

    class Generator {
        public:
            Generator() = default;
            virtual ~Generator() = default;
            virtual void nextBlock(uint8_t* buffer, size_t size) = 0;
    };

    class ConstantGenerator : public Generator {
        public:
            void nextBlock(uint8_t* buffer, size_t size) override {
                std::memset(buffer, 0, size);
            }
    };

    class RandomGenerator : public Generator {
        private:
            prng_state generator;

        public:
            RandomGenerator();
            void nextBlock(uint8_t* buffer, size_t size) override;
    };
};

#endif