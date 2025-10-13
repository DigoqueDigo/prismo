#include <generator/block.h>

namespace Generator {

    void BlockConfig::validate(void) const {
        if (size % BLOCK_BASE_SIZE != 0)
            throw std::invalid_argument("Invalid block size, must be multiple of " + std::to_string(BLOCK_BASE_SIZE));
    }

    Block::Block(const BlockConfig& _config)
        : size(_config.getSize()), buffer(nullptr) {
            buffer = static_cast<uint8_t*>(std::malloc(_config.getSize()));
            if (buffer == nullptr) {
                throw std::bad_alloc();
            }
        }

    Block::~Block() {
        std::free(buffer);
    }

    void from_json(const json& j, BlockConfig& config) {
        j.at("block_size").get_to(config.size);
        config.validate();
    }
}