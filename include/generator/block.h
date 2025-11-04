#ifndef BLOCK_H
#define BLOCK_H

#include <new>
#include <cstdint>
#include <cstdlib>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

#define BLOCK_BASE_SIZE 128

namespace Generator {

    class Block {
        private:
            size_t size;
            uint8_t* buffer;

        public:
            Block() = default;
            ~Block();

            constexpr inline size_t getSize(void) const { return size; };
            constexpr inline uint8_t* getBuffer(void) { return buffer; };

            void validate(void) const;
            friend void from_json(const json& j, Block& config);
        };

    void from_json(const json& j, Block& config);
};

#endif