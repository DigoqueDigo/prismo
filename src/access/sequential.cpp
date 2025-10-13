#include <access/sequential.h>
#include <stdexcept>

namespace Access {

    void SequentialAccessConfig::validate(void) const {
        if (block_size == 0)
            throw std::invalid_argument("Invalid block_size for SequentialAccessConfig");
        if (block_size > limit)
            throw std::invalid_argument("Invalid limit for SequentialAccessConfig");
    }

    SequentialAccess::SequentialAccess(const SequentialAccessConfig& _config)
        : config(_config), current_offset(0) {}

    size_t SequentialAccess::nextOffset(void) {
        const size_t offset = current_offset;
        current_offset = (current_offset + config.getBlockSize()) % config.getLimit();
        return offset;
    }

    void from_json(const json& j, SequentialAccessConfig& config) {
        j.at("block_size").get_to(config.block_size);
        j.at("limit").get_to(config.limit);
        config.validate();
        config.limit = config.block_size * (config.limit / config.block_size);
    }
}
