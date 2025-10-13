#ifndef BLOCK_H
#define BLOCK_H

#include <new>
#include <cstdint>
#include <cstdlib>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

#define BLOCK_BASE_SIZE 128

namespace Generator {

    struct BlockConfig {
        private:
            size_t size;

        public:
            constexpr inline size_t getSize(void) const { return size; }

            void validate(void) const;
            friend void from_json(const json& j, BlockConfig& config);
    };

    struct Block {
        private:
        const size_t size;
        uint8_t* buffer;

        public:
            explicit Block(const BlockConfig& _config);
            ~Block();
        
            // Prevent copying (to avoid double free)
            Block(const Block&) = delete;
            Block& operator=(const Block&) = delete;

            constexpr inline size_t getSize(void) const { return size; };
            constexpr inline uint8_t* getBuffer(void) { return buffer; };
    };

    void from_json(const json& j, BlockConfig& config);
};

#endif