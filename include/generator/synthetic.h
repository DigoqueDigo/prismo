#ifndef SYNTHETIC_GENERATOR_H
#define SYNTHETIC_GENERATOR_H

#include <cstdint>
#include <cstddef>
#include <cstring>
#include <iostream>
#include <unordered_map>
#include <nlohmann/json.hpp>
#include <lib/distribution/distribution.h>

using json = nlohmann::json;

namespace Generator {

    class Generator {
        protected:
            uint64_t block_id = 0;

        public:
            Generator() = default;

            virtual ~Generator() {
                // std::cout << "~Destroying Generator" << std::endl;
            }

            virtual uint64_t nextBlock(uint8_t* buffer, size_t size) = 0;
    };

    class ConstantGenerator : public Generator {
        public:
            ConstantGenerator() = default;

            ~ConstantGenerator() override {
                // std::cout << "~Destroying ConstantGenerator" << std::endl;
            }

            uint64_t nextBlock(uint8_t* buffer, size_t size) override {
                std::memset(buffer, 0, size);
                std::memcpy(buffer, &block_id, sizeof(block_id));
                return block_id;
            }
    };

    class RandomGenerator : public Generator {
        private:
            prng_state generator;

        public:
            RandomGenerator();

            ~RandomGenerator() override {
                // std::cout << "~Destroying RandomGenerator" << std::endl;
            }

            uint64_t nextBlock(uint8_t* buffer, size_t size) override;
    };

    struct WindowElement {
        uint64_t block_id;
        uint32_t left_copies;
    };

    class DedupGenerator : public Generator {
        private:
            size_t sliding_window = 2;
            Distribution::UniformDistribution<uint32_t> distribution;

            std::vector<std::pair<uint32_t, uint32_t>> percentages;
            std::unordered_map<uint32_t, std::vector<WindowElement>> windows;

        public:
            DedupGenerator();

            ~DedupGenerator() override {
                // std::cout << "~Destroying DedupGenerator" << std::endl;
            }

            uint64_t nextBlock(uint8_t* buffer, size_t size) override;
            friend void from_json(const json& j, DedupGenerator& generator);
    };

    void from_json(const json& j, DedupGenerator& generator);
};

#endif
