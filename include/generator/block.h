#ifndef BLOCK_H
#define BLOCK_H

#include <new>
#include <cstddef>
#include <cstdlib>
#include <stdexcept>

namespace BlockGenerator {
    struct Block {
        const size_t size;
        std::byte* buffer;

        explicit Block(size_t _size)
            : size(_size), buffer(nullptr) {
                buffer = static_cast<std::byte*>(std::malloc(size));
                if (buffer == nullptr) {
                    throw std::bad_alloc();
                }
            }

        constexpr inline size_t getSize() const {
            return size;
        }

        constexpr inline const std::byte* getBuffer() const {
            return buffer;
        }

        ~Block() {
            std::free(buffer);
        }   
    };
};

#endif