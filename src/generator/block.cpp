#include <generator/block.h>

namespace Generator {

    Block::~Block() {
        std::free(buffer);
    }

    void Block::validate(void) const {
        if (size % BLOCK_BASE_SIZE != 0)
            throw std::invalid_argument("Invalid block size, must be multiple of " + std::to_string(BLOCK_BASE_SIZE));
    }

    void from_json(const json& j, Block& config) {
        j.at("block_size").get_to(config.size);
        config.validate();
        config.buffer = static_cast<uint8_t*>(std::malloc(config.size));
        if (config.buffer == nullptr) {
            throw std::bad_alloc();
        }
    }
}