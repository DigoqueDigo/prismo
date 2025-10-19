#include <access/random.h>
#include <stdexcept>

namespace Access {

    off_t RandomAccess::nextOffset(void) {
        return static_cast<off_t>(distribution.nextValue() * block_size);
    }

    void RandomAccess::validate(void) const {
        if (block_size == 0)
            throw std::invalid_argument("Invalid block_size for RandomAccessConfig");
        if (block_size > limit)
            throw std::invalid_argument("Invalid limit for RandomAccessConfig");
    }

    void from_json(const json& j, RandomAccess& config) {
        j.at("block_size").get_to(config.block_size);
        j.at("limit").get_to(config.limit);
        config.validate();
        config.limit = config.limit / config.block_size - 1;
        config.distribution.setParams(0, config.limit);
    }
}
