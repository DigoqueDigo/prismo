#ifndef BLOCK_H
#define BLOCK_H

#include <new>
#include <cstdint>
#include <cstdlib>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

#define BLOCK_BASE_SIZE 128

namespace BlockGenerator {
    struct BlockConfig {
        size_t size;

        void validate(void) const {
            if (size % BLOCK_BASE_SIZE != 0)
                throw std::invalid_argument("Invalid block size, must be multiple of " + std::to_string(BLOCK_BASE_SIZE));
        };
    };

    struct Block {
        const BlockConfig config;
        uint8_t* buffer;

        explicit Block(const BlockConfig& _config)
            : config(_config), buffer(nullptr) {
                buffer = static_cast<uint8_t*>(std::malloc(_config.size));
                if (buffer == nullptr) {
                    throw std::bad_alloc();
                }
            }

        ~Block() {
            std::free(buffer);
        }   
    };

    void from_json(const json& j, BlockConfig& config) {
        j.at("block_size").get_to(config.size);
        config.validate();
    }
};

#endif