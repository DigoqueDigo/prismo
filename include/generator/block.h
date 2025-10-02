#ifndef BLOCK_H
#define BLOCK_H

#include <new>
#include <cstdint>
#include <cstdlib>

namespace BlockGenerator {
    struct Block {
        const size_t size;
        uint8_t* buffer;

        explicit Block(size_t _size)
            : size(_size), buffer(nullptr) {
                buffer = static_cast<uint8_t*>(std::malloc(size));
                if (buffer == nullptr) {
                    throw std::bad_alloc();
                }
            }

        ~Block() {
            std::free(buffer);
        }   
    };
};

#endif