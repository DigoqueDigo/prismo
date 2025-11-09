#ifndef SYNTHETIC_GENERATOR_H
#define SYNTHETIC_GENERATOR_H

#include <cstdint>
#include <cstddef>
#include <cstring>
#include <iostream>
#include <lib/shishua/shishua.h>

namespace Generator {

    class Generator {
        public:
            Generator() = default;

            virtual ~Generator() {
                std::cout << "Destroying Generator" << std::endl;
            }

            virtual void nextBlock(uint8_t* buffer, size_t size) = 0;
    };

    class ConstantGenerator : public Generator {
        public:
            ConstantGenerator() = default;

            ~ConstantGenerator() override {
                std::cout << "Destroying ConstantGenerator" << std::endl;
            }

            void nextBlock(uint8_t* buffer, size_t size) override {
                std::memset(buffer, 0, size);
            }
    };

    class RandomGenerator : public Generator {
        private:
            prng_state generator;

        public:
            RandomGenerator();

            ~RandomGenerator() override {
                std::cout << "Destroying RandomGenerator" << std::endl;
            }

            void nextBlock(uint8_t* buffer, size_t size) override;
    };
};

#endif
